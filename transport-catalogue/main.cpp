#include "geo.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "test.h"

using namespace std::string_literals;

int main() {
	//***!!!***
	// This part is used for testing.
	// 
	//TestReadInput();
	//TestFillTransportCatalogueIstream();
	//TestFillTransportCatalogueJson();
	//std::cout << "Test complete!!!"s;
	// 
	//***!!!***

	request_handler::Read(std::cin, request_handler::KindOfStream::json);
}
