#include "json_reader.h"


using namespace std::literals;

namespace input_json {

	Json_reader::Json_reader(std::istream& input) : document_(json::Load(input)){
	}

    transport::TransportCatalogue Json_reader::MakeTransportCatalogue() {
        transport::TransportCatalogue res;
        const json::Node& root = document_.GetRoot();
        const json::Node& base_requests = root.AsDict().at("base_requests");
        std::map<std::pair<std::string, std::string>, int> vector_info_distance_stop;
		size_t count = 0;
        for (auto& request : base_requests.AsArray()) {
            if (request.AsDict().at("type").AsString() == "Stop") {
                std::string name = request.AsDict().at("name").AsString();
                double latitude = request.AsDict().at("latitude").AsDouble();
                double longitude = request.AsDict().at("longitude").AsDouble();
                for (auto& [stop_name, distance] : request.AsDict().at("road_distances").AsDict()) {
                    vector_info_distance_stop[std::pair{ name, stop_name }] = distance.AsDouble();
                    if (!vector_info_distance_stop.count(std::pair{ stop_name, name })) {
                        vector_info_distance_stop[std::pair{ stop_name, name }] = distance.AsDouble();
                    }
                }
				domain::Stop stop{ name, latitude, longitude, count };
				++count;
				res.AddStop(stop);
            }
        }

        for (auto info : vector_info_distance_stop) {
            res.AddDistanceBetweenStops(info.first.first, info.first.second, info.second);
        }

        for (auto& request : base_requests.AsArray()) {
            if (request.AsDict().at("type").AsString() == "Bus") {
                std::string bus_number = request.AsDict().at("name").AsString();
                bool round = request.AsDict().at("is_roundtrip").AsBool();

                std::vector<std::string> stops;
                for (auto& stop : request.AsDict().at("stops").AsArray()) {
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
		const json::Node& stat_requests = root.AsDict().at("stat_requests"s);
		json::Array respone;
		for (auto& stat_request : stat_requests.AsArray()) {
			if (stat_request.AsDict().at("type"s).AsString() == "Stop"sv)
			{
				try {
					auto info = request.InfoStop(stat_request.AsDict().at("name"s).AsString());
					json::Array buses;
					for (auto bus : info) {
						buses.emplace_back(static_cast<std::string>(bus));
					}
					respone.emplace_back(
						json::Builder{}
						.StartDict()
						.Key("buses"s).Value(buses)
						.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
						.EndDict()
						.Build());
			}
				catch (std::out_of_range const&) {
					respone.emplace_back(
						json::Builder{}
						.StartDict()
						.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
						.EndDict()
						.Build());
				}
			}
			else if (stat_request.AsDict().at("type"s).AsString() == "Bus"sv) 
			{
				auto info = request.InfoBus(stat_request.AsDict().at("name"s).AsString());
				if (info.Find) {
					respone.emplace_back(
						json::Builder{}
						.StartDict()
						.Key("curvature"s).Value(std::round(info.crookedness * 100000) / 100000)
						.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
						.Key("route_length"s).Value(std::round(info.Lenght * 100000) / 100000)
						.Key("stop_count"s).Value(static_cast<int>(info.Size))
						.Key("unique_stop_count"s).Value(info.UniqueStop)
						.EndDict()
						.Build());
				}
				else {
					respone.emplace_back(
						json::Builder{}
						.StartDict()
						.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
						.Key("error_message"s).Value("not found"s)
						.EndDict()
						.Build());
				}

			}
			else if (stat_request.AsDict().at("type"s).AsString() == "Map"sv)
			{
				auto output = request.Render();
				respone.emplace_back(
					json::Builder{}
					.StartDict()
					.Key("map"s).Value(output)
					.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
					.EndDict()
					.Build());
			}
			else if (stat_request.AsDict().at("type"s).AsString() == "Route"sv)
			{
				auto route_info = request.FasterRoute(stat_request.AsDict().at("from"s).AsString(), stat_request.AsDict().at("to"s).AsString());
				if (route_info.has_value()) 
				{
					auto respone_item_builder = json::Builder{};
					auto respone_item_dict = respone_item_builder.StartDict();

					auto items_array = respone_item_dict.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
						.Key("total_time"s).Value(route_info.value().total_time)
						.Key("items"s).StartArray();
					
					for(auto& item : route_info.value().items) {
						std::visit([&items_array](auto&& arg) {
							using T = std::decay_t<decltype(arg)>;
							if constexpr (std::is_same_v<T, domain::RouteItemWait>) {
								items_array.StartDict()
									.Key("stop_name"s).Value(static_cast<std::string>(arg.stop_name))
									.Key("time"s).Value(arg.time)
									.Key("type"s).Value("Wait"s)
									.EndDict();
							}
							else if constexpr (std::is_same_v<T, domain::RouteItemBus>) {
								items_array.StartDict()
									.Key("bus"s).Value(static_cast<std::string>(arg.bus))
									.Key("span_count"s).Value(arg.span_count)
									.Key("time"s).Value(arg.time)
									.Key("type"s).Value("Bus"s)
									.EndDict();
							}
							else {
								static_assert(always_false_v<T>, "non-exhaustive visitor!");
							}
							}, item);
					}
					items_array.EndArray();
					respone_item_dict.EndDict();
					respone.emplace_back(std::move(respone_item_builder.Build()));
				}
				else 
				{
					respone.emplace_back(
						json::Builder{}
						.StartDict()
						.Key("error_message"s).Value("not found"s)
						.Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
						.EndDict()
						.Build());
				}
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

	domain::ParametersMap Json_reader::MakeParametersForMap() {
		domain::ParametersMap parametrs;
		const json::Node& root = document_.GetRoot();
		const json::Dict& render_settings = root.AsDict().at("render_settings"s).AsDict();

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

		parametrs.padding = render_settings.at("padding"s).AsDouble();

		return parametrs;
	}

	const json::Node& Json_reader::GetRoot() const {
		return document_.GetRoot();
	}

	domain::RoutingSettings Json_reader::MakeRoutingSetting() {
		const json::Node& root = document_.GetRoot();
		const json::Dict& routing_settings = root.AsDict().at("routing_settings"s).AsDict();
		domain::RoutingSettings result;
		if (routing_settings.count("bus_wait_time"s)) {
			result.bus_wait_time = routing_settings.at("bus_wait_time"s).AsInt();
		}
		if (routing_settings.count("bus_velocity"s)) {
			result.bus_velocity = routing_settings.at("bus_velocity"s).AsInt();

		}
		return result;
	}
}
