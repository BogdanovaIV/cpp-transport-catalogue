#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <sstream>


namespace input_json {

	transport::TransportCatalogue MakeTransportCatalogue(const json::Document& document);

	void Reguest(transport::TransportCatalogue& TCatalogue, json::Document& document);

	std::pair<json::Document, transport::TransportCatalogue> ReadStream(std::istream& input);

	svg::Color MakeColor(const json::Node&);

	map_renderer::map MakeParametersForMap(json::Document& document, const transport::TransportCatalogue& TCatalogue);
}