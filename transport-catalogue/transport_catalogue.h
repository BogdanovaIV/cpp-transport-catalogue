#pragma once
#include<deque>
#include<string>
#include<string_view>
#include<vector>
#include<unordered_map>
#include<algorithm>
#include<stdexcept>
#include "geo.h"
#include <iterator>
#include <set>


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


class TransportCatalogue {
	struct Stop {
		std::string name;
		double latitude;
		double longitude;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> Route;

	};

public:
	TransportCatalogue() {}

	TransportCatalogue(TransportCatalogue& TCatalogue) {
		std::swap(Stops, TCatalogue.Stops);
		std::swap(Stops_to_Name, TCatalogue.Stops_to_Name);
		std::swap(Buses, TCatalogue.Buses);
		std::swap(Buses_to_Name, TCatalogue.Buses_to_Name);
	}

	void AddStop(std::string& name, double latitude, double longitude);
	const Stop* FindStop(const std::string& name) const;
	void AddBus(std::string& BusName, std::vector<std::string>& BusStops, bool round);
	const Bus* FindBus(const std::string& BusName) const;
	BusInformation InfoBus(const std::string& BusName);
	std::set<std::string_view> InfoStop(const std::string& Stop);
	void AddDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond, int distance);
	int FindDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond);
	int FindDistanceBetweenStops(const Stop* StopFirst, const Stop* StopSecond);

	void GetRouteInfo();
private:

	struct DistanceBetweenStopsHasher {
		size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
			return static_cast<size_t>(stops.first->latitude * 1000 + stops.first->longitude * 1000 * 13 + stops.second->latitude * 1000 * 13 * 13 + stops.second->longitude * 1000 * 13 * 13 * 13);
		}
	};

	struct DistanceBetweenStopsEqual {
		bool operator() (const std::pair<const Stop*, const Stop*>& left, const std::pair<const Stop*, const Stop*>& right) const {
			return left.first == right.first && left.second == right.second;
		}

	};

	std::deque<Stop> Stops;
	std::unordered_map<std::string_view, Stop*> Stops_to_Name;
	std::deque<Bus> Buses;
	std::unordered_map<std::string_view, Bus*> Buses_to_Name;
	std::unordered_map<std::string_view, std::set<std::string_view>> Stop_to_Buses;
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceBetweenStopsHasher, DistanceBetweenStopsEqual> DistanceBetweenStops;

};