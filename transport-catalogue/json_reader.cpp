#include "json_reader.h"

using namespace std::literals;

namespace input_json {

	Json_reader::Json_reader(std::istream& input) : document_(json::Load(input)){
	}

    transport::TransportCatalogue Json_reader::MakeTransportCatalogue() {
        transport::TransportCatalogue res;
        const json::Node& root = document_.GetRoot();
        const json::Node& base_requests = root.AsMap().at("base_requests");
        std::map<std::pair<std::string, std::string>, int> vector_info_distance_stop;

        for (auto& request : base_requests.AsArray()) {
            if (request.AsMap().at("type").AsString() == "Stop") {
                std::string name = request.AsMap().at("name").AsString();
                double latitude = request.AsMap().at("latitude").AsDouble();
                double longitude = request.AsMap().at("longitude").AsDouble();
                for (auto& [stop_name, distance] : request.AsMap().at("road_distances").AsMap()) {
                    vector_info_distance_stop[std::pair{ name, stop_name }] = distance.AsDouble();
                    if (!vector_info_distance_stop.count(std::pair{ stop_name, name })) {
                        vector_info_distance_stop[std::pair{ stop_name, name }] = distance.AsDouble();
                    }
                }
				domain::Stop stop{ name, latitude, longitude };
				res.AddStop(stop);
            }
        }

        for (auto info : vector_info_distance_stop) {
            res.AddDistanceBetweenStops(info.first.first, info.first.second, info.second);
        }

        for (auto& request : base_requests.AsArray()) {
            if (request.AsMap().at("type").AsString() == "Bus") {
                std::string bus_number = request.AsMap().at("name").AsString();
                bool round = request.AsMap().at("is_roundtrip").AsBool();

                std::vector<std::string> stops;
                for (auto& stop : request.AsMap().at("stops").AsArray()) {
                    stops.push_back(stop.AsString());
                }
                res.AddBus(bus_number, stops, round);
            }
        }

        return res;
    }

	void Json_reader::Reguest(request_handler::Request& request)
	{
		const json::Node& root = document_.GetRoot();
		const json::Node& stat_requests = root.AsMap().at("stat_requests"s);
		json::Array respone;
		for (auto& stat_request : stat_requests.AsArray()) {
			if (stat_request.AsMap().at("type"s).AsString() == "Stop"sv) {
				try {
					auto info = request.InfoStop(stat_request.AsMap().at("name"s).AsString());
					json::Array buses;
					for (auto bus : info) {
						buses.emplace_back(static_cast<std::string>(bus));
					}
					respone.emplace_back(
					    json::Dict {
						{"buses"s, buses},
						{"request_id"s, stat_request.AsMap().at("id"s).AsInt()},
					});
			}
				catch (std::out_of_range const&) {
					respone.emplace_back(
						json::Dict{
						{"request_id"s, stat_request.AsMap().at("id"s).AsInt()},
						{"error_message"s, "not found"s}
					});
				}
			}
			else if (stat_request.AsMap().at("type"s).AsString() == "Bus"sv) {
				auto info = request.InfoBus(stat_request.AsMap().at("name"s).AsString());
				if (info.Find) {
					respone.emplace_back(
						json::Dict{
						{"curvature"s, std::round(info.crookedness * 100000) / 100000},
						{"request_id"s, stat_request.AsMap().at("id"s).AsInt()},
						{"route_length"s, std::round(info.Lenght * 100000) / 100000},
						{"stop_count"s, static_cast<int>(info.Size)},
						{"unique_stop_count"s, info.UniqueStop}
					});
				}
				else {
					respone.emplace_back(
						json::Dict{
						{"request_id"s, stat_request.AsMap().at("id"s).AsInt()},
						{"error_message"s, "not found"s}
					});
				}

			}
			else if (stat_request.AsMap().at("type"s).AsString() == "Map"sv) {
				auto output = request.Render();
				respone.emplace_back(
					json::Dict{
					{"map"s, output},
					{"request_id"s, stat_request.AsMap().at("id"s).AsInt()}
					});
			}
		}
		json::Print(json::Document{ respone }, std::cout);

	}

	svg::Color Json_reader::MakeColor(const json::Node& settings) {
		
		if (settings.IsString()) {
			return svg::Color{ settings.AsString() };
		}
		else if (settings.IsArray()) {
			if (settings.AsArray().size() == 3) {
				return svg::Color{ svg::Rgb{
				static_cast<unsigned int>(settings.AsArray().at(0).AsInt()),
				static_cast<unsigned int>(settings.AsArray().at(1).AsInt()),
				static_cast<unsigned int>(settings.AsArray().at(2).AsInt())
				} };
			}
			else if (settings.AsArray().size() == 4) {
				return svg::Color{ svg::Rgba{
					static_cast<unsigned int>(settings.AsArray().at(0).AsInt()),
					static_cast<unsigned int>(settings.AsArray().at(1).AsInt()),
					static_cast<unsigned int>(settings.AsArray().at(2).AsInt()),
					settings.AsArray().at(3).AsDouble()
				} };
			}
		}
		return svg::Color{};
	}

	std::pair<domain::ParametersMap, double> Json_reader::MakeParametersForMap() {
		domain::ParametersMap parametrs;
		const json::Node& root = document_.GetRoot();
		const json::Dict& render_settings = root.AsMap().at("render_settings"s).AsMap();

		parametrs.width = render_settings.at("width"s).AsDouble();
		parametrs.height = render_settings.at("height"s).AsDouble();
		parametrs.line_width = render_settings.at("line_width"s).AsDouble();
		parametrs.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsDouble();
		parametrs.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsDouble();
		parametrs.stop_radius = render_settings.at("stop_radius"s).AsDouble();
		parametrs.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();

		if (render_settings.at("bus_label_offset"s).AsArray().size() == 2) {
			parametrs.bus_label_offset.first = render_settings.at("bus_label_offset"s).AsArray()[0].AsDouble();
			parametrs.bus_label_offset.second = render_settings.at("bus_label_offset"s).AsArray()[1].AsDouble();
		}

		if (render_settings.at("stop_label_offset"s).AsArray().size() == 2) {
			parametrs.stop_label_offset.first = render_settings.at("stop_label_offset"s).AsArray()[0].AsDouble();
			parametrs.stop_label_offset.second = render_settings.at("stop_label_offset"s).AsArray()[1].AsDouble();
		}
		parametrs.underlayer_color = MakeColor(render_settings.at("underlayer_color"s));
		for (auto color : render_settings.at("color_palette"s).AsArray()) {
			parametrs.color_palette.push_back(MakeColor(color));
		}

		return { parametrs, render_settings.at("padding"s).AsDouble() };
	}
	const json::Node& Json_reader::GetRoot() const {
		return document_.GetRoot();
	}
}
