#include <fstream>
#include <filesystem>
#include "geo.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>

using namespace std::literals;

namespace serilization {

	void Serialize(std::string path_string, transport::TransportCatalogue& TCatalogue,
		domain::ParametersMap& parameters, domain::RoutingSettings& routing_setings);

	transport::TransportCatalogue DeserializeTransportCatalogue(std::string& path_string);
	domain::ParametersMap DeserializeParametersMap(std::string& path_string);
	domain::RoutingSettings DeserializeRoutingSetting(std::string& path_string);

}