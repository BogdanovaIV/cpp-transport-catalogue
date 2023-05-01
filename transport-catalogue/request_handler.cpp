#include "request_handler.h"

using namespace std::literals;

namespace request_handler {
	Request::Request(transport::TransportCatalogue&& TCatalogue, domain::ParametersMap& parameters, domain::RoutingSettings&& routing_setting) :
		TCatalogue_(std::move(TCatalogue), static_cast<double>(routing_setting.bus_velocity) * 1000 / 60, routing_setting.bus_wait_time), render_(map_renderer::Renderer(parameters)), routing_setting_(std::move(routing_setting)) {

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
		return render_.Render(TCatalogue_.GetAllStopsWithCoordinates());
	}

	std::optional<domain::RouteInfo> Request::FasterRoute(std::string from, std::string to) {
		return TCatalogue_.FindFastestRoute(TCatalogue_.FindStop(from)->id, TCatalogue_.FindStop(to)->id);
	}
}
