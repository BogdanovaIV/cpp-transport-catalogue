#include "request_handler.h"

using namespace std::literals;

namespace request_handler {
	Request::Request(transport::TransportCatalogue TCatalogue, domain::ParametersMap& parameters, double padding) :
		TCatalogue_(std::move(TCatalogue)), render_(map_renderer::Renderer(parameters, padding, TCatalogue_.GetAllStopsWithCoordinates())) {

	}

	std::set<std::string_view> Request::InfoStop(const std::string& Name) {
		std::set<std::string_view> result;
		try {
			if (TCatalogue_.FindStop(Name)) {
				result = TCatalogue_.InfoStop(Name);
			}
		}
		catch (std::out_of_range const&) {
			throw std::out_of_range("Don't find stop."s);
		}
		return result;
	}

	domain::BusInformation Request::InfoBus(const std::string& BusName) {
		return TCatalogue_.InfoBus(BusName);
	}

	std::string Request::Render() {
		return render_.Render();
	}
}
