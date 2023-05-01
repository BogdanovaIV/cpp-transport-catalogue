#include "transport_router.h"

namespace transport {
    TransportRoutes::TransportRoutes() :
        TransportCatalogue()
    {
    }

    TransportRoutes::TransportRoutes(double speed_meters,
        int waiting_time_at_station_minute) :
        TransportCatalogue(),
        speed_meters_(speed_meters),
        waiting_time_at_station_minute_(waiting_time_at_station_minute)
    {
    }

    TransportRoutes::TransportRoutes(transport::TransportCatalogue& TCatalogue, double speed_meters,
        int waiting_time_at_station_minute) :
        TransportCatalogue(TCatalogue),
        speed_meters_(speed_meters),
        waiting_time_at_station_minute_(waiting_time_at_station_minute)
    {
    }

    TransportRoutes::TransportRoutes(transport::TransportCatalogue&& TCatalogue, double speed_meters,
        int waiting_time_at_station_minute) :
        TransportCatalogue(std::move(TCatalogue)),
        speed_meters_(speed_meters),
        waiting_time_at_station_minute_(waiting_time_at_station_minute)
    {
    }

    std::optional<domain::RouteInfo> TransportRoutes::FindFastestRoute(size_t from, size_t to) {
        if (router_.get() == nullptr) {
            BuildRouteGraph();
        }
        const auto route_info = router_->BuildRoute(from, to);
        if (route_info.has_value()) {
            domain::RouteInfo result;
            for (const auto& it : route_info.value().edges) {
                auto& edge = Graph_.GetEdge(it);
                result.items.push_back(domain::RouteItemWait{ edge.name , static_cast<double>(waiting_time_at_station_minute_) });
                result.items.push_back(domain::RouteItemBus{ edge.bus, edge.span_count, edge.weight - waiting_time_at_station_minute_ });

                result.total_time += edge.weight;
            }

            return result;
        }
        return std::nullopt;

    }

    size_t TransportRoutes::RouteStopsHasher::operator()(const domain::Route& route) const {
        static const uint32_t kMultiplier = 13;
        static const uint32_t kBase = 1000;
        uint32_t h = 0;
        h = h * kMultiplier + static_cast<uint32_t>(route.from->latitude * kBase);
        h = h * kMultiplier + static_cast<uint32_t>(route.from->longitude * kBase);
        h = h * kMultiplier + static_cast<uint32_t>(route.to->latitude * kBase);
        h = h * kMultiplier + static_cast<uint32_t>(route.to->longitude * kBase);
        return static_cast<size_t>(h);
    }

    bool TransportRoutes::RouteStopsEqual::operator() (const domain::Route& left, const domain::Route& right) const {
        return (left.from == right.from) && (left.to == right.to);
    }

    void TransportRoutes::BuildRouteGraph() {
        TRoute routes;
        auto& buses = GetBuses();
        routes.reserve(buses.size() * 20);
        for (const auto& bus : buses) {
            AddInRoutes(bus.Route.begin(), bus.Route.end(), bus, routes);
            if (!bus.is_roundtrip) {
                AddInRoutes(bus.Route.rbegin(), bus.Route.rend(), bus, routes);
            }
        }
        Graph_.Resize(GetSizeStops());
        for (const auto& [route, bus] : routes) {
            Graph_.AddEdge({ route.from->id, route.to->id, route.time, std::string_view(route.from->name), std::string_view(bus->name),  route.span_count });
        }
        router_ = std::make_unique<graph::Router<double>>(Graph_);
    }
}