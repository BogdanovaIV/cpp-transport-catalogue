#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include "request_handler.h"
#include <map>

namespace input_istream {
	std::string ReadLine(std::istream& input);

	int ReadLineWithNumber(std::istream& input);

	std::vector<std::pair<transport::detail::KindOfRequest, std::string>> Read(std::istream& input, bool FillCatalogue);

	transport::TransportCatalogue MakeTransportCatalogue(const std::vector<std::pair<transport::detail::KindOfRequest, std::string>>& vec_request);

	void Reguest(transport::TransportCatalogue& TCatalogue, std::vector<std::pair<transport::detail::KindOfRequest, std::string>>& vec_request_out);

	void ReadStream(std::istream& input);

}