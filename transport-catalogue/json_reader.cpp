#include "json_reader.h"

using namespace std::literals;

namespace input_json {
    transport::TransportCatalogue MakeTransportCatalogue(const json::Document& document) {
        transport::TransportCatalogue res;
        const json::Node& root = document.GetRoot();
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
                res.AddStop(name, latitude, longitude);
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

	void Reguest(transport::TransportCatalogue& TCatalogue, json::Document& document)
	{
		const json::Node& root = document.GetRoot();
		const json::Node& stat_requests = root.AsMap().at("stat_requests"s);
		json::Array respone;
		for (auto& request : stat_requests.AsArray()) {
			if (request.AsMap().at("type"s).AsString() == "Stop"sv) {
				try {
					if (TCatalogue.FindStop(request.AsMap().at("name"s).AsString())) {
						auto info = TCatalogue.InfoStop(request.AsMap().at("name"s).AsString());
						json::Array buses;
						for (auto bus : info) {
							buses.emplace_back(static_cast<std::string>(bus));
						}
						respone.emplace_back(
					        json::Dict {
							{"buses"s, buses},
							{"request_id"s, request.AsMap().at("id"s).AsInt()},
						});
					}
				}
				catch (std::out_of_range const&) {
					respone.emplace_back(
						json::Dict{
						{"request_id"s, request.AsMap().at("id"s).AsInt()},
						{"error_message"s, "not found"s}
					});
				}
			}
			else if (request.AsMap().at("type"s).AsString() == "Bus"sv) {
				auto info = TCatalogue.InfoBus(request.AsMap().at("name"s).AsString());
				if (info.Find) {
					respone.emplace_back(
						json::Dict{
						{"curvature"s, std::round(info.crookedness * 100000) / 100000},
						{"request_id"s, request.AsMap().at("id"s).AsInt()},
						{"route_length"s, std::round(info.Lenght * 100000) / 100000},
						{"stop_count"s, static_cast<int>(info.Size)},
						{"unique_stop_count"s, info.UniqueStop}
					});
				}
				else {
					respone.emplace_back(
						json::Dict{
						{"request_id"s, request.AsMap().at("id"s).AsInt()},
						{"error_message"s, "not found"s}
					});
				}

			}
			else if (request.AsMap().at("type"s).AsString() == "Map"sv) {
				auto map = input_json::MakeParametersForMap(document, TCatalogue);
				std::ostringstream output;
				map.Render(output);
				respone.emplace_back(
					json::Dict{
					{"map"s, output.str()},
					{"request_id"s, request.AsMap().at("id"s).AsInt()}
					});
			}
		}
		json::Print(json::Document{ respone }, std::cout);

	}

	std::pair<json::Document, transport::TransportCatalogue> ReadStream(std::istream& input) {

        json::Document document = json::Load(input);

        transport::TransportCatalogue TCatalogue = MakeTransportCatalogue(document);
        
		return { document, TCatalogue };		
	}

	svg::Color MakeColor(const json::Node& settings) {
		
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

	map_renderer::map MakeParametersForMap(json::Document& document, const transport::TransportCatalogue& TCatalogue) {
		std::unordered_map<std::string, double> parametrs;
		parametrs.reserve(8);
		const json::Node& root = document.GetRoot();
		const json::Dict& render_settings = root.AsMap().at("render_settings"s).AsMap();
		
		parametrs[std::move("width"s)] = render_settings.at("width"s).AsDouble();
		parametrs[std::move("height"s)] = render_settings.at("height"s).AsDouble();
		parametrs[std::move("line_width"s)] = render_settings.at("line_width"s).AsDouble();
		parametrs[std::move("bus_label_font_size"s)] = render_settings.at("bus_label_font_size"s).AsDouble();
		parametrs[std::move("stop_label_font_size"s)] = render_settings.at("stop_label_font_size"s).AsDouble();
		parametrs[std::move("stop_radius"s)] = render_settings.at("stop_radius"s).AsDouble();
		parametrs[std::move("underlayer_width"s)] = render_settings.at("underlayer_width"s).AsDouble();
		parametrs[std::move("padding"s)] = render_settings.at("padding"s).AsDouble();
		
		std::pair<double, double> bus_label_offset{0.0, 0.0};
		if (render_settings.at("bus_label_offset"s).AsArray().size() == 2) {
			bus_label_offset.first = render_settings.at("bus_label_offset"s).AsArray()[0].AsDouble();
			bus_label_offset.second = render_settings.at("bus_label_offset"s).AsArray()[1].AsDouble();
		}
		
		std::pair<double, double> stop_label_offset{0.0, 0.0};
		if (render_settings.at("stop_label_offset"s).AsArray().size() == 2) {
			stop_label_offset.first = render_settings.at("stop_label_offset"s).AsArray()[0].AsDouble();
			stop_label_offset.second = render_settings.at("stop_label_offset"s).AsArray()[1].AsDouble();
		}
		svg::Color underlayer_color = MakeColor(render_settings.at("underlayer_color"s));
		std::vector<svg::Color> color_palette;
		color_palette.reserve(render_settings.at("color_palette"s).AsArray().size());
		for (auto color : render_settings.at("color_palette"s).AsArray()) {
			color_palette.push_back(MakeColor(color));
		}

		const std::deque<transport::TransportCatalogue::Bus>& buses_deque = TCatalogue.GetBuses();
		
		std::vector<transport::TransportCatalogue::Bus*> buses;
		buses.reserve(buses_deque.size());
		for (auto& bus : buses_deque) {
			buses.push_back(&const_cast<transport::TransportCatalogue::Bus&>(bus));
		}
		std::sort(buses.begin(), buses.end(), [](transport::TransportCatalogue::Bus* lhs, transport::TransportCatalogue::Bus* rhs) {return lhs->name < rhs->name; });

		std::vector<geo::Coordinates> coordinates;
		std::vector<std::pair<const transport::TransportCatalogue::Stop*, const transport::TransportCatalogue::Bus*>> Stops_Buses;
		for (const auto& bus : buses) {
			for (const auto& stop : bus->Route) {
				coordinates.push_back({stop->latitude, stop->longitude});
				Stops_Buses.push_back({ stop, bus });
			}
		}
		map_renderer::map res(parametrs, bus_label_offset, stop_label_offset, underlayer_color, color_palette, coordinates, Stops_Buses);
		return res;
	}
}
