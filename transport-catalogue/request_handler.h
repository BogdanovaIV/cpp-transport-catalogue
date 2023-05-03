#pragma once
#include <iostream>
#include <iomanip>
#include <optional>
#include "map_renderer.h"
#include "transport_router.h"


namespace request_handler {

    class Request{
    public:
        Request(transport::TransportCatalogue&& TCatalogue, domain::ParametersMap& parameters, domain::RoutingSettings&& routing_setting);

        std::set<std::string_view> InfoStop(const std::string& Name);
    
        domain::BusInformation InfoBus(const std::string& BusName);

        std::string Render();

        std::optional<domain::RouteInfo> FasterRoute(std::string from, std::string to);

    private:
        transport::TransportCatalogue TCatalogue_;
        std::optional<transport::TransportRoutes> TRoutes_;
        map_renderer::Renderer render_;
        domain::RoutingSettings routing_setting_;
        graph::DirectedWeightedGraph<double> Graph_;
    };

}
