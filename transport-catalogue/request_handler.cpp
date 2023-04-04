#include "request_handler.h"

using namespace std::literals;

void request_handler::Read(std::istream& input, const KindOfStream& kind_stream) {
	if (kind_stream == KindOfStream::istream) {
		input_istream::ReadStream(input);
	}
	else if (kind_stream == KindOfStream::json) {
		auto [document, TCatalogue] = input_json::ReadStream(input);
		input_json::Reguest(TCatalogue, document);

	}
}

void request_handler::Draw(std::istream& input, const KindOfStream& kind_stream) {
	if (kind_stream == KindOfStream::json) {
		auto [document, TCatalogue] = input_json::ReadStream(input);
		auto map = input_json::MakeParametersForMap(document, TCatalogue);
		map.Render(std::cout);
	}
}