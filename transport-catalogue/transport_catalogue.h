#pragma once
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

		struct BusInformation {
			bool Find;
			size_t Size = 0;
			int UniqueStop = 0;
			double Lenght = 0.0;
			double crookedness = 0.0;
		};

	}

	using namespace detail;

	class TransportCatalogue {
	public:

		struct Stop {
			std::string name;
			double latitude;
			double longitude;
		};

		struct Bus {
			std::string name;
			std::vector<const Stop*> Route;
			const Stop* stop_begin = nullptr;
			const Stop* stop_end = nullptr;
			bool is_roundtrip = false;
		};

		TransportCatalogue() {}

		TransportCatalogue(TransportCatalogue& TCatalogue) {
			std::swap(Stops, TCatalogue.Stops);
			std::swap(Stops_to_Name, TCatalogue.Stops_to_Name);
			std::swap(Buses, TCatalogue.Buses);
			std::swap(Buses_to_Name, TCatalogue.Buses_to_Name);
			std::swap(Stop_to_Buses, TCatalogue.Stop_to_Buses);
			std::swap(DistanceBetweenStops, TCatalogue.DistanceBetweenStops);
		}

		void AddStop(std::string& name, double latitude, double longitude);
		const Stop* FindStop(const std::string& name) const;
		std::set<std::string_view> InfoStop(const std::string& Stop);

		void AddBus(std::string& BusName, std::vector<std::string>& BusStops, bool round);
		const Bus* FindBus(const std::string& BusName) const;
		BusInformation InfoBus(const std::string& BusName);
		
		void AddDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond, int distance);
		int FindDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond);
		int FindDistanceBetweenStops(const Stop* StopFirst, const Stop* StopSecond);
		
		const std::deque<Bus>& GetBuses() const;
	private:

		struct DistanceBetweenStopsHasher {
			size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const;
		};

		struct DistanceBetweenStopsEqual {
			bool operator() (const std::pair<const Stop*, const Stop*>& left, const std::pair<const Stop*, const Stop*>& right) const;
		};

		std::deque<Stop> Stops;
		std::unordered_map<std::string_view, Stop*> Stops_to_Name;
		std::deque<Bus> Buses;
		std::unordered_map<std::string_view, Bus*> Buses_to_Name;
		std::unordered_map<std::string_view, std::set<std::string_view>> Stop_to_Buses;
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceBetweenStopsHasher, DistanceBetweenStopsEqual> DistanceBetweenStops;

	};
}
