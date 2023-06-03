#include "serialization.h"

namespace serilization {
	void MakeBase() {
		input_json::Json_reader json_reader{ std::cin };
		transport::TransportCatalogue TCatalogue = json_reader.MakeTransportCatalogue();
		
		const std::filesystem::path path(json_reader.GetSerializationSettingsFile());
		if (path.empty()) {
			std::cout << "File name didn't find."sv;
			return;
		}
		transport_catalogue_serialize::TransportCatalogue STransportCatalogue = CreateTransportCatalogue(TCatalogue);
		std::ofstream out_clear(path, std::ios::binary);
		out_clear.clear();
		out_clear.close();

		std::ofstream out_file(path, std::ios::binary | std::ios::app);
		STransportCatalogue.SerializeToOstream(&out_file);

		domain::ParametersMap parameters = json_reader.MakeParametersForMap();
		map_renderer_serialize::ParametersMap SParametersMap = CreateParametersMap(parameters);
		SParametersMap.SerializeToOstream(&out_file);

		domain::RoutingSettings routing_setings = json_reader.MakeRoutingSetting();
		svg_serialize::RoutingSettings SRoutingSettings = CreateRoutingSettings(routing_setings);
		SRoutingSettings.SerializeToOstream(&out_file);
		out_file.close();

	}

	transport_catalogue_serialize::TransportCatalogue CreateTransportCatalogue(transport::TransportCatalogue& TCatalogue) {
		transport_catalogue_serialize::TransportCatalogue STCatalogue;
		for (auto& stop : TCatalogue.GetStops()) {
			transport_catalogue_serialize::Stops* Sstop = STCatalogue.add_stops();
			Sstop->set_name(stop.name);
			Sstop->set_latitude(stop.latitude);
			Sstop->set_longitude(stop.longitude);
			Sstop->set_id(stop.id);
		}
		
		for (auto& bus : TCatalogue.GetBuses()) {
			transport_catalogue_serialize::Buses* Sbus = STCatalogue.add_buses();
			Sbus->set_name(bus.name);
			Sbus->set_id(bus.id);
			Sbus->set_is_roundtrip(bus.is_roundtrip);
			Sbus->set_stop_begin_id(bus.stop_begin->id);
			Sbus->set_stop_end_id(bus.stop_end->id);
			for (auto& stop : bus.Route) {
				Sbus->add_route(stop->id);
			}
		}

		for (auto [stops, distance] : TCatalogue.GetDistanceBetweenStops()) {
			transport_catalogue_serialize::DistanceBetweenStops* SDistanceBetweenStops = STCatalogue.add_distance();
			SDistanceBetweenStops->set_id_stop1(stops.first->id);
			SDistanceBetweenStops->set_id_stop2(stops.second->id);
			SDistanceBetweenStops->set_distance(distance);
		}
		return STCatalogue;
	}

	map_renderer_serialize::ParametersMap CreateParametersMap(domain::ParametersMap& parameters) {
		map_renderer_serialize::ParametersMap SParametersMap;
		SParametersMap.set_width(parameters.width);
		SParametersMap.set_height(parameters.height);
		SParametersMap.set_line_width(parameters.line_width);
		SParametersMap.set_stop_radius(parameters.stop_radius);
		SParametersMap.set_bus_label_font_size(parameters.bus_label_font_size);
		SParametersMap.set_bus_label_offset1(parameters.bus_label_offset.first);
		SParametersMap.set_bus_label_offset2(parameters.bus_label_offset.second);
		SParametersMap.set_stop_label_font_size(parameters.stop_label_font_size);
		SParametersMap.set_stop_label_offset1(parameters.stop_label_offset.first);
		SParametersMap.set_stop_label_offset2(parameters.stop_label_offset.second);
		
		struct ColorSerialeze {
			map_renderer_serialize::Color& Scolor;

			void operator()(std::monostate) const {
				Scolor.set_type(0);
			}
			void operator()(std::string color) const {
				Scolor.set_type(1);
				Scolor.set_name(color);
			}
			void operator()(svg::Rgb color) const {
				Scolor.set_type(2);
				Scolor.set_r(color.red);
				Scolor.set_g(color.green);
				Scolor.set_b(color.blue);
			}
			void operator()(svg::Rgba color) const {
				Scolor.set_type(3);
				Scolor.set_r(color.red);
				Scolor.set_g(color.green);
				Scolor.set_b(color.blue);
				Scolor.set_a(color.opacity);
			}
		};
		map_renderer_serialize::Color color;
		std::visit(ColorSerialeze{ color }, parameters.underlayer_color);
		*SParametersMap.mutable_underlayer_color() = std::move(color);
		
		SParametersMap.set_underlayer_width(parameters.underlayer_width);

		for (auto& pcolor : parameters.color_palette) {
			map_renderer_serialize::Color* color = SParametersMap.add_color_palette();
			std::visit(ColorSerialeze{ *color }, pcolor);
		}

		SParametersMap.set_padding(parameters.padding);

		return SParametersMap;
	}

	svg_serialize::RoutingSettings CreateRoutingSettings(domain::RoutingSettings& routing_setings) {
		svg_serialize::RoutingSettings SRoutingSettings;
		SRoutingSettings.set_bus_velocity(routing_setings.bus_velocity);
		SRoutingSettings.set_bus_wait_time(routing_setings.bus_wait_time);
		return SRoutingSettings;
	}

	void ProcessRequests() {
		input_json::Json_reader json_reader{ std::cin };
		const std::filesystem::path path(json_reader.GetSerializationSettingsFile());
		auto parameters = DeserializeParametersMap(path);

		request_handler::Request request(DeserializeTransportCatalogue(path), parameters, DeserializeRoutingSetting(path));
		json_reader.Reguest(request);
	}

	transport::TransportCatalogue DeserializeTransportCatalogue(const std::filesystem::path& path) {
		std::ifstream in_file(path, std::ios::binary);
		transport::TransportCatalogue TCatalogue;
		transport_catalogue_serialize::TransportCatalogue object;
		if (object.ParseFromIstream(&in_file)) {
			std::map<int, std::string> map_stops;

			for (auto& Sstop : object.stops()) {
				domain::Stop stop{ Sstop.name(), Sstop.latitude(), Sstop.longitude(), static_cast<size_t>(Sstop.id())};
				map_stops.emplace(stop.id, stop.name);
				TCatalogue.AddStop(stop);
			}

			for (auto& SDistance : object.distance()) {
				TCatalogue.AddDistanceBetweenStops(map_stops.at(SDistance.id_stop1()), map_stops.at(SDistance.id_stop2()), SDistance.distance());
			}

			for (auto& bus : object.buses()) {
				std::string bus_number = bus.name();
		
				std::vector<std::string> stops;
				for (auto& stop : bus.route()) {
					stops.push_back(map_stops.at(stop));
				}
				TCatalogue.AddBus(bus_number, stops, bus.is_roundtrip(), static_cast<size_t>(bus.id()));
			}
		}
		
		return TCatalogue;
	}

	svg::Color MakeColor(const map_renderer_serialize::Color& Scolor) {
		if (Scolor.type() == 0) {
			return svg::Color{};
		}

		if (Scolor.type() == 1) {
			return svg::Color{ Scolor.name()};
		}

		if (Scolor.type() == 2) {
			return svg::Color{ svg::Rgb{Scolor.r(), Scolor.g(), Scolor.b()}};
		}

		if (Scolor.type() == 3) {
			return svg::Color{ svg::Rgba{Scolor.r(), Scolor.g(), Scolor.b(), Scolor.a()} };
		}

		return svg::Color{};
	}


	domain::ParametersMap DeserializeParametersMap(const std::filesystem::path& path) {
		std::ifstream in_file(path, std::ios::binary);
		domain::ParametersMap ParametersMap;
		map_renderer_serialize::ParametersMap object;
		if (object.ParseFromIstream(&in_file)) {

			ParametersMap.width = object.width();
			ParametersMap.height = object.height();
			ParametersMap.line_width = object.line_width();
			ParametersMap.stop_radius = object.stop_radius();
			ParametersMap.bus_label_font_size = object.bus_label_font_size();
			ParametersMap.bus_label_offset = std::pair{ object.bus_label_offset1(),object.bus_label_offset2() };
			ParametersMap.stop_label_font_size = object.stop_label_font_size();
			ParametersMap.stop_label_offset = std::pair{ object.stop_label_offset1(),object.stop_label_offset2() };
			ParametersMap.underlayer_color = MakeColor(object.underlayer_color());
			ParametersMap.underlayer_width = object.underlayer_width();

			for (auto& Scolor : object.color_palette()) {
				ParametersMap.color_palette.push_back(MakeColor(Scolor));
			}

			ParametersMap.padding = object.padding();

		}

		return ParametersMap;

	}

	domain::RoutingSettings DeserializeRoutingSetting(const std::filesystem::path& path) {
		std::ifstream in_file(path, std::ios::binary);
		domain::RoutingSettings RoutingSettings;
		svg_serialize::RoutingSettings object;
		if (object.ParseFromIstream(&in_file)) {
			RoutingSettings.bus_velocity = object.bus_velocity();
			RoutingSettings.bus_wait_time = object.bus_wait_time();
		}
		return RoutingSettings;
	}
}