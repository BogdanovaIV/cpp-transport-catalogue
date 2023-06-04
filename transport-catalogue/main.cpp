#include"serialization.h"
#include <fstream>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase() {
	input_json::Json_reader json_reader{ std::cin };
	transport::TransportCatalogue TCatalogue = json_reader.MakeTransportCatalogue();
	domain::ParametersMap parameters = json_reader.MakeParametersForMap();
	domain::RoutingSettings routing_setings = json_reader.MakeRoutingSetting();

	serilization::Serialize(json_reader.GetSerializationSettingsFile(), TCatalogue, parameters, routing_setings);
}

void ProcessRequests() {
    input_json::Json_reader json_reader{ std::cin };
    std::string path_string = json_reader.GetSerializationSettingsFile();
    auto parameters = serilization::DeserializeParametersMap(path_string);

    request_handler::Request request(serilization::DeserializeTransportCatalogue(path_string), parameters, serilization::DeserializeRoutingSetting(path_string));
    json_reader.Reguest(request);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        MakeBase();
    } else if (mode == "process_requests"sv) {

        ProcessRequests();

    } else {
        PrintUsage();
        return 1;
    }
}