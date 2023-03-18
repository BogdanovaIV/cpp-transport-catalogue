#include "stat_reader.h"

using namespace std::string_literals;

void output::Reguest(TransportCatalogue& TCatalogue, std::vector<std::pair<KindOfRequest, std::string>>& vec_request_out) {
	for (auto& [kind_request, request] : vec_request_out) {
		if (kind_request == KindOfRequest::InfoBus) {
			auto info = TCatalogue.InfoBus(request);
			if (info.Find) {
				std::cout << "Bus "s << request << ": ";
				std::cout << info.Size << " stops on route, ";
				std::cout << info.UniqueStop << " unique stops, ";
				std::cout << std::setprecision(6) << info.Lenght << " route length, ";
				std::cout << std::setprecision(6) << info.crookedness << " curvature" << std::endl;

			}
			else {
				std::cout << "Bus "s << request << ": "; 
				std::cout << "not found"  <<std::endl;
			}
		}
		else if (kind_request == KindOfRequest::InfoStop) {
			try {
				if (TCatalogue.FindStop(request)) {
					auto info = TCatalogue.InfoStop(request);
					if (info.empty()) {
						std::cout << "Stop "s << request << ": ";
						std::cout << "no buses" << std::endl;

					}
					else {
						std::cout << "Stop "s << request << ": buses";
						for (auto bus : info) {
							std::cout << " "s << bus;
						}
						std::cout << std::endl;
					}
				}
			}
			catch (std::out_of_range const&) {
				std::cout << "Stop "s << request << ": ";
				std::cout << "not found" << std::endl;
			}
		}
	}
}