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

	namespace detail {

		enum KindOfRequest {
			AddStop = 1,
			AddBus,
			InfoBus,
			InfoStop
		};

	}

	using namespace detail;

	class TransportCatalogue {
		struct DistanceBetweenStopsHasher;
		struct DistanceBetweenStopsEqual;
	public:

		TransportCatalogue() {}

		TransportCatalogue(TransportCatalogue& TCatalogue);

		TransportCatalogue(TransportCatalogue&& TCatalogue);

		void AddStop(domain::Stop& stop);
		const domain::Stop* FindStop(const std::string& name) const;
		const domain::Stop* FindStop(std::string_view name) const;
		std::set<std::string_view> InfoStop(std::string_view Stop);
		std::set<std::string_view> InfoStop(const std::string& Stop);

		void AddBus(std::string& BusName, std::vector<std::string>& BusStops, bool round, size_t id);
		const domain::Bus* FindBus(std::string_view BusName) const;
		const domain::Bus* FindBus(const std::string& BusName) const;
		domain::BusInformation InfoBus(const std::string& BusName);

		void AddDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond, int distance);
		int FindDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond);
		int FindDistanceBetweenStops(const domain::Stop* StopFirst, const domain::Stop* StopSecond);

		const std::deque<domain::Bus>& GetBuses() const;
		const std::deque<domain::Stop>& GetStops() const;
		const std::unordered_map<std::string_view, std::set<std::string_view>>& GetStop_to_Buses() const;
		const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, DistanceBetweenStopsHasher, DistanceBetweenStopsEqual>& GetDistanceBetweenStops() const;

		std::pair<std::vector<geo::Coordinates>, std::vector<std::pair<const domain::Stop*, const domain::Bus*>>> GetAllStopsWithCoordinates();

		const domain::Stop* GetStopByIndex(int index) const;

		size_t GetSizeStops() const;

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
