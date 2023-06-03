#include "transport_router.h"

namespace transport {

    TransportRoutes::TransportRoutes(transport::TransportCatalogue& TCatalogue, domain::RoutingSettings RoutingSettings_) :
        speed_meters_(static_cast<double>(RoutingSettings_.bus_velocity) * 1000 / 60),
        waiting_time_at_station_minute_(RoutingSettings_.bus_wait_time)
    {
        BuildRouteGraph(TCatalogue);
    }

    TransportRoutes::TransportRoutes(TransportRoutes&& other) {
        router_ = std::move(other.router_);
        Graph_ = std::move(other.Graph_);
        std::swap(speed_meters_, other.speed_meters_);
        std::swap(waiting_time_at_station_minute_, other.waiting_time_at_station_minute_);
    }

    std::optional<domain::RouteInfo> TransportRoutes::FindFastestRoute(size_t from, size_t to) {
        const auto route_info = router_->BuildRoute(from, to);
        if (route_info.has_value()) {
            domain::RouteInfo result;
            for (const auto& it : route_info.value().edges) {
                auto& edge = Graph_.get()->GetEdge(it);
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

    void TransportRoutes::BuildRouteGraph(transport::TransportCatalogue& TCatalogue) {
        TRoute routes;
        auto& buses = TCatalogue.GetBuses();
        routes.reserve(buses.size() * 20);
        for (const auto& bus : buses) {
            AddInRoutes(TCatalogue, bus.Route.begin(), bus.Route.end(), bus, routes);
            if (!bus.is_roundtrip) {
                AddInRoutes(TCatalogue, bus.Route.rbegin(), bus.Route.rend(), bus, routes);
            }
        }
        graph::DirectedWeightedGraph<double> Graph(TCatalogue.GetSizeStops());
        for (const auto& [route, bus] : routes) {
            Graph.AddEdge({ route.from->id, route.to->id, route.time, std::string_view(route.from->name), std::string_view(bus->name),  route.span_count });
        }

        Graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(Graph);
        router_ = std::make_unique<graph::Router<double>>(*Graph_.get());
    }
}