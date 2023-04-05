#pragma once
#include "domain.h"
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include "geo.h"
#include <iterator>
#include <set>

namespace transport {
	
	namespace detail{

		enum KindOfRequest {
			AddStop = 1,
			AddBus,
			InfoBus,
			InfoStop
		};

	}

	using namespace detail;

	class TransportCatalogue {
	public:

		TransportCatalogue() {}

		TransportCatalogue(TransportCatalogue& TCatalogue);

		TransportCatalogue(TransportCatalogue&& TCatalogue);

		void AddStop(domain::Stop& stop);
		const domain::Stop* FindStop(const std::string& name) const;
		std::set<std::string_view> InfoStop(const std::string& Stop);

		void AddBus(std::string& BusName, std::vector<std::string>& BusStops, bool round);
		const domain::Bus* FindBus(const std::string& BusName) const;
		domain::BusInformation InfoBus(const std::string& BusName);
		
		void AddDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond, int distance);
		int FindDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond);
		int FindDistanceBetweenStops(const domain::Stop* StopFirst, const domain::Stop* StopSecond);
		
		const std::deque<domain::Bus>& GetBuses() const;

		std::pair<std::vector<geo::Coordinates>, std::vector<std::pair<const domain::Stop*, const domain::Bus*>>> GetAllStopsWithCoordinates();

	private:

		struct DistanceBetweenStopsHasher {
			size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const;
		};

		struct DistanceBetweenStopsEqual {
			bool operator() (const std::pair<const domain::Stop*, const domain::Stop*>& left, const std::pair<const domain::Stop*, const domain::Stop*>& right) const;
		};

		std::deque<domain::Stop> Stops;
		std::unordered_map<std::string_view, domain::Stop*> Stops_to_Name;
		std::deque<domain::Bus> Buses;
		std::unordered_map<std::string_view, domain::Bus*> Buses_to_Name;
		std::unordered_map<std::string_view, std::set<std::string_view>> Stop_to_Buses;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, DistanceBetweenStopsHasher, DistanceBetweenStopsEqual> DistanceBetweenStops;

	};
}
