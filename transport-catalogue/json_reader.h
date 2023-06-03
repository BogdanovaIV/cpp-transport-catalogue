#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "request_handler.h"
#include <type_traits>

template<class>
inline constexpr bool always_false_v = false;

namespace input_json {

	class Json_reader {
	public:

		Json_reader(std::istream& input);
		transport::TransportCatalogue MakeTransportCatalogue();

		void Reguest(request_handler::Request& request);

		svg::Color MakeColor(const json::Node&);

		domain::ParametersMap MakeParametersForMap();

		const json::Node& GetRoot() const;

		domain::RoutingSettings MakeRoutingSetting();

		std::string GetSerializationSettingsFile();

	private:

		json::Document document_;

	};
}