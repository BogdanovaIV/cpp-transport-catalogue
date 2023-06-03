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

	void MakeBase();

	transport_catalogue_serialize::TransportCatalogue CreateTransportCatalogue(transport::TransportCatalogue& TCatalogue);
	map_renderer_serialize::ParametersMap CreateParametersMap(domain::ParametersMap& parameters);
	svg_serialize::RoutingSettings CreateRoutingSettings(domain::RoutingSettings& routing_setings);

	void ProcessRequests();

	transport::TransportCatalogue DeserializeTransportCatalogue(const std::filesystem::path& path);	
	domain::ParametersMap DeserializeParametersMap(const std::filesystem::path& path);
	domain::RoutingSettings DeserializeRoutingSetting(const std::filesystem::path& path);

}