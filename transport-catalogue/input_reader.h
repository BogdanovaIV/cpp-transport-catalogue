#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include "stat_reader.h"
#include <map>

namespace input_istream {
	std::string ReadLine(std::istream& input);

	int ReadLineWithNumber(std::istream& input);

	std::vector<std::pair<KindOfRequest, std::string>> Read(std::istream& input, bool FillCatalogue);

	TransportCatalogue FillTransportCatalogue(const std::vector<std::pair<KindOfRequest, std::string>>& vec_request);

	void ReadStream(std::istream& input);

}