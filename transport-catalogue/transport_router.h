#pragma once
#include "graph.h"
#include "transport_catalogue.h"
#include "router.h"
#include <unordered_set>
#include <queue>

namespace transport {
 
 
    class TransportRoutes {
	public:
 
        TransportRoutes(transport::TransportCatalogue& TCatalogue, domain::RoutingSettings RoutingSettings_);

        TransportRoutes(TransportRoutes && other);

        std::optional<domain::RouteInfo> FindFastestRoute(size_t from, size_t to);

    private:
         
         std::unique_ptr <graph::DirectedWeightedGraph<double>> Graph_;
         std::unique_ptr <graph::Router<double>> router_;
         double speed_meters_ = 0;
         int waiting_time_at_station_minute_ = 0;

     
         struct RouteStopsHasher {
             size_t operator()(const domain::Route& route) const;
         };

         struct RouteStopsEqual {
             bool operator() (const domain::Route& left, const domain::Route& right) const;
         };

         using TRoute = std::unordered_map<domain::Route, const domain::Bus*, RouteStopsHasher, RouteStopsEqual>;

         template <typename Iterator>
         void AddInRoutes(transport::TransportCatalogue& TCatalogue, Iterator Begin, Iterator End, const domain::Bus& bus, TRoute& routes);

         void BuildRouteGraph(transport::TransportCatalogue& TCatalogue);

    };

    template <typename Iterator>
    void TransportRoutes::AddInRoutes(transport::TransportCatalogue& TCatalogue, Iterator Begin, Iterator End, const domain::Bus& bus, TRoute& routes) {
        for (auto from_stop = Begin; from_stop != (End - 1); ++from_stop) {
            double weight = 0.;
            int span_count = 0;
            const domain::Stop* from_stop_dist = *from_stop;
            for (auto to_stop = (from_stop + 1); to_stop != End; ++to_stop) {
                weight += TCatalogue.FindDistanceBetweenStops(from_stop_dist, *to_stop);
                if (from_stop_dist != *to_stop) {
                    ++span_count;
                }
                from_stop_dist = *to_stop;
                const domain::Stop* from_stop_pointer = *from_stop;
                const domain::Stop* to_stop_pointer = *to_stop;

                double time = weight / speed_meters_ + waiting_time_at_station_minute_;
                auto it = routes.find(domain::Route{ from_stop_pointer, to_stop_pointer });
                if (it != routes.end()) {
                    if ((*it).first.time > time) {
                        auto route = const_cast<domain::Route*>(&(it->first));
                        route->time = time;
                        route->span_count = span_count;
                        (*it).second = &bus;
                    }

                }
                else
                {
                    routes.emplace(domain::Route{ *from_stop, *to_stop, time, span_count }, &bus);
                }
            }
        }
    }


}