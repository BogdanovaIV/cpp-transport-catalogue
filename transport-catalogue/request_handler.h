#pragma once
#include <iostream>
#include <iomanip>
#include "map_renderer.h"

namespace request_handler {

    class Request{
    public:
        Request(transport::TransportCatalogue&& TCatalogue, domain::ParametersMap& parameters);

        std::set<std::string_view> InfoStop(const std::string& Name);
    
        domain::BusInformation InfoBus(const std::string& BusName);

        std::string Render();

    private:
        transport::TransportCatalogue TCatalogue_;
        map_renderer::Renderer render_;
    };

}
