#include "geo.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "test.h"

using namespace std::string_literals;

int main() {
	//***!!!***
	// This part is used for testing.
	// 
	//TestFillTransportCatalogueJson();
	//std::cout << "Test complete!!!"s;
	// 
	//***!!!***
	input_json::Json_reader json_reader{ std::cin };
	auto parameters = json_reader.MakeParametersForMap();
	request_handler::Request request(json_reader.MakeTransportCatalogue(), parameters);
	json_reader.Reguest(request);
}
