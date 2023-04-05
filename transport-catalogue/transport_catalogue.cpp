#include "transport_catalogue.h"

using namespace std::string_literals;

transport::TransportCatalogue::TransportCatalogue(TransportCatalogue& TCatalogue) {
	std::swap(Stops, TCatalogue.Stops);
	std::swap(Stops_to_Name, TCatalogue.Stops_to_Name);
	std::swap(Buses, TCatalogue.Buses);
	std::swap(Buses_to_Name, TCatalogue.Buses_to_Name);
	std::swap(Stop_to_Buses, TCatalogue.Stop_to_Buses);
	std::swap(DistanceBetweenStops, TCatalogue.DistanceBetweenStops);
}

transport::TransportCatalogue::TransportCatalogue(TransportCatalogue&& TCatalogue) {
	Stops = std::move(TCatalogue.Stops);
	Stops_to_Name = std::move(TCatalogue.Stops_to_Name);
	Buses = std::move(TCatalogue.Buses);
	Buses_to_Name = std::move(TCatalogue.Buses_to_Name);
	Stop_to_Buses = std::move(TCatalogue.Stop_to_Buses);
	DistanceBetweenStops = std::move(TCatalogue.DistanceBetweenStops);
}

size_t transport::TransportCatalogue::DistanceBetweenStopsHasher::operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const {
	return static_cast<size_t>(stops.first->latitude * 1000 + stops.first->longitude * 1000 * 13 + stops.second->latitude * 1000 * 13 * 13 + stops.second->longitude * 1000 * 13 * 13 * 13);
}

bool transport::TransportCatalogue::DistanceBetweenStopsEqual::operator() (const std::pair<const domain::Stop*, const domain::Stop*>&left, const std::pair<const domain::Stop*, const domain::Stop*>&right) const {
	return left.first == right.first && left.second == right.second;
}

void transport::TransportCatalogue::AddStop(domain::Stop& stop) {

	Stops.push_back(std::move(stop));
	domain::Stop* stop_pointer = &Stops.back();
	Stops_to_Name.insert({ stop_pointer->name, stop_pointer });
}

const domain::Stop* transport::TransportCatalogue::FindStop(const std::string& name) const{
	return Stops_to_Name.at(name);
}

std::set<std::string_view> transport::TransportCatalogue::InfoStop(const std::string& Stop) {

	try {
		return Stop_to_Buses.at(Stop);
	}
	catch (std::out_of_range const&) {
		return std::set<std::string_view>{};
	}
}

void transport::TransportCatalogue::AddBus(std::string& BusName, std::vector<std::string>& BusStops, bool round) {
	size_t size = (round) ? BusStops.size() : 2 * BusStops.size() - 1;
	std::vector<const domain::Stop*> Route(size);
	const domain::Stop* stop_begin = nullptr;
	const domain::Stop* stop_end = nullptr;
	int i = 0;
	for (auto& stop : BusStops) {
		const domain::Stop* stop_pointer = FindStop(stop);
		Route[i] = stop_pointer;
		if (stop_begin == nullptr) {
			stop_begin = stop_pointer;
		}
		stop_end = stop_pointer;
		if (!round) {
			Route[size - i - 1] = stop_pointer;
		}
		++i;
	}
	
	domain::Bus bus{ std::move(BusName), std::move(Route), stop_begin, stop_end, round };
	Buses.push_back(std::move(bus));
	domain::Bus* bus_pointer = &Buses.back();
	for (auto stop: bus_pointer->Route) {
		Stop_to_Buses[stop->name].emplace(bus_pointer->name);
	}
	Buses_to_Name.insert({ bus_pointer->name, bus_pointer });
}

const domain::Bus* transport::TransportCatalogue::FindBus(const std::string& BusName) const {
	return Buses_to_Name.at(BusName);
}

const std::deque<domain::Bus>& transport::TransportCatalogue::GetBuses() const {
	return Buses;
}

domain::BusInformation transport::TransportCatalogue::InfoBus(const std::string& BusName) {
	domain::BusInformation res;
	try {
		auto Bus = FindBus(BusName);
		res.Find = true;
		res.Size = Bus->Route.size();
		res.Lenght = 0;
		std::vector<const domain::Stop*> vec_copy(res.Size);
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

void transport::TransportCatalogue::AddDistanceBetweenStops(const std::string & StopFirst, const std::string & StopSecond, int distance) {
	DistanceBetweenStops[std::pair{ FindStop(StopFirst), FindStop(StopSecond) }] = distance;
}

int transport::TransportCatalogue::FindDistanceBetweenStops(const std::string& StopFirst, const std::string& StopSecond) {
	return DistanceBetweenStops.at(std::pair{ FindStop(StopFirst), FindStop(StopSecond) });
}

int transport::TransportCatalogue::FindDistanceBetweenStops(const domain::Stop * StopFirst, const domain::Stop * StopSecond) {
	return DistanceBetweenStops.at(std::pair{StopFirst, StopSecond});
}

std::pair<std::vector<geo::Coordinates>, std::vector<std::pair<const domain::Stop*, const domain::Bus*>>> transport::TransportCatalogue::GetAllStopsWithCoordinates() {
	std::vector<domain::Bus*> buses_vector;
	buses_vector.reserve(Buses.size());
	for (auto& bus : Buses) {
		buses_vector.push_back(&const_cast<domain::Bus&>(bus));
	}
	std::sort(buses_vector.begin(), buses_vector.end(), [](domain::Bus* lhs, domain::Bus* rhs) {return lhs->name < rhs->name; });

	std::vector<geo::Coordinates> coordinates;
	std::vector<std::pair<const domain::Stop*, const domain::Bus*>> Stops_Buses;
	for (const auto& bus : buses_vector) {
		for (const auto& stop : bus->Route) {
			coordinates.push_back({ stop->latitude, stop->longitude });
			Stops_Buses.push_back({ stop, bus });
		}
	}
	return { coordinates , Stops_Buses };
}