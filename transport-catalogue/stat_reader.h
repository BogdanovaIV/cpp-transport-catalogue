#pragma once
#include "transport_catalogue.h"
#include <iostream>
#include <iomanip>

namespace output {
	void Reguest(transport::TransportCatalogue& TCatalogue, std::vector<std::pair<transport::detail::KindOfRequest, std::string>>& vec_request_out);
}
