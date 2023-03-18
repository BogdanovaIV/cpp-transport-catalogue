#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <iomanip>

namespace output {
	void Reguest(TransportCatalogue& TCatalogue, std::vector<std::pair<KindOfRequest, std::string>>& vec_request_out);
}
