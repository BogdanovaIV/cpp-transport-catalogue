#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "request_handler.h"

namespace input_json {

	class Json_reader {
	public:

		Json_reader(std::istream& input);
		transport::TransportCatalogue MakeTransportCatalogue();

		void Reguest(request_handler::Request& request);

		svg::Color MakeColor(const json::Node&);

		domain::ParametersMap MakeParametersForMap();

		const json::Node& GetRoot() const;

	private:

		json::Document document_;

	};
}