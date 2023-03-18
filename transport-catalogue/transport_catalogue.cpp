#include "transport_catalogue.h"

using namespace std::string_literals;

void TransportCatalogue::AddStop(std::string& name, double latitude, double longitude) {

	Stop stop{ std::move(name), latitude, longitude };
	Stops.push_back(std::move(stop));
	Stop* stop_pointer = &Stops.back();
	Stops_to_Name.insert({ stop_pointer->name, stop_pointer });
}

const TransportCatalogue::Stop* TransportCatalogue::FindStop(const std::string& name) const{
	return Stops_to_Name.at(name);
}

void TransportCatalogue::AddBus(std::string& BusName, std::vector<std::string>& BusStops, bool round) {
	size_t size = (round) ? BusStops.size() : 2 * BusStops.size() - 1;
	std::vector<const Stop*> Route(size);
	int i = 0;
	for (auto& stop : BusStops) {
		const Stop* stop_pointer = FindStop(stop);
		Route[i] = stop_pointer;
		if (!round) {
			Route[size - i - 1] = stop_pointer;
		}
		++i;
	}
	
	Bus bus{ std::move(BusName), std::move(Route) };
	Buses.push_back(std::move(bus));
	Bus* bus_pointer = &Buses.back();
	for (auto stop: bus_pointer->Route) {
		Stop_to_Buses[stop->name].emplace(bus_pointer->name);
	}
	Buses_to_Name.insert({ bus_pointer->name, bus_pointer });
}

const TransportCatalogue::Bus* TransportCatalogue::FindBus(const std::string& BusName) const {
	return Buses_to_Name.at(BusName);
}

BusInformation TransportCatalogue::InfoBus(const std::string& BusName) {
	BusInformation res;
	try {
		auto Bus = FindBus(BusName);
		res.Find = true;
		res.Size = Bus->Route.size();
		res.Lenght = 0;
		std::vector<const Stop*> vec_copy(res.Size);
		std::copy(Bus->Route.begin(), Bus->Route.end(), vec_copy.begin());
		auto first = vec_copy.begin();
		auto second = first + 1;
		for (auto& stop = second; stop < vec_copy.end(); ++stop) {
			auto stop_ref = *stop;
			auto first_ref = *first;
			auto crookedness = geo::ComputeDistance({first_ref->latitude, first_ref->longitude}, {stop_ref->latitude, stop_ref->longitude});
			auto Lenght = FindDistanceBetweenStops(first_ref, stop_ref);
			res.Lenght += Lenght;
			res.crookedness += crookedness;
			first = stop;
		}
		res.crookedness = res.Lenght / res.crookedness;
		std::sort(vec_copy.begin(), vec_copy.end());
		res.UniqueStop = std::unique(vec_copy.begin(), vec_copy.end()) - vec_copy.begin();
	}
	catch (std::out_of_range const&) {
		res.Find = false;
	}

	return res;
}

std::set<std::string_view> TransportCatalogue::InfoStop(const std::string& Stop) {
	
	try {
		return Stop_to_Buses.at(Stop);
	}
	catch (std::out_of_range const&) {
		return std::set<std::string_view>{};
	}
}

void TransportCatalogue::AddDistanceBetweenStops(const std::string & StopFirst, const std::string & StopSecond, int distance) {
	DistanceBetweenStops[std::pair{ FindStop(StopFirst), FindStop(StopSecond) }] = distance;
}

int TransportCatalogue::FindDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond) {
	return DistanceBetweenStops.at(std::pair{ FindStop(StopFirst), FindStop(StopSecond) });
}

int TransportCatalogue::FindDistanceBetweenStops(const Stop * StopFirst, const Stop * StopSecond) {
	return DistanceBetweenStops.at(std::pair{StopFirst, StopSecond});
	}