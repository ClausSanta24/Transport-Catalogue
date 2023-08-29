#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(stop);
	stopname_to_stop_.insert({ stops_.back().name, &stops_.back() });
}

Stop& TransportCatalogue::FindStop(const std::string& stop) {
	if (stopname_to_stop_.count(stop) > 0) {
		return *stopname_to_stop_.find(stop)->second;
	}
	return const_cast<Stop&>(EMPTY_STOP);
}

void TransportCatalogue::AddBus(const Bus& bus) {
	buses_.push_back(bus);
	busname_to_bus_.insert({ buses_.back().name, &buses_.back() });
	for (const auto& stop : bus.bus) {
		busname_to_stop_[stop].push_back(&buses_.back());
	}
}

Bus& TransportCatalogue::FindBus(const std::string& bus) {
	if (busname_to_bus_.count(bus) > 0) {
		return *busname_to_bus_.find(bus)->second;
	}
	return const_cast<Bus&>(EMPTY_BUS);
}

Bus& TransportCatalogue::GetBusInfo(const std::string& bus) {
	return FindBus(bus);
}

std::vector<Bus*> TransportCatalogue::GetBusesByStop(const std::string& stop) {
	if (busname_to_stop_.count(&FindStop(stop)) > 0) {
		auto temp = busname_to_stop_.find(&FindStop(stop))->second;
        std::sort(temp.begin(), temp.end(), [](Bus* left, Bus* right) {
			return left->name < right->name;
            });
		temp.erase(std::unique(temp.begin(), temp.end()), temp.end());
		return temp;
	}
	return EMPTY_VEC_BUS;
}

void TransportCatalogue::AddDistance(Stop* left, Stop* right, double distance) {
	distances_[{left, right}] = distance;
}

double TransportCatalogue::GetDistance(Stop* left, Stop* right) {
	if (distances_.count({ left, right }) > 0) {
		return distances_.at({ left, right });
	}
	return 0;
}

std::vector<Bus*> TransportCatalogue::GetAllBuses() {
	std::vector<Bus*> result;
	for (Bus& bus : buses_) {
		result.push_back(&bus);
	}
	return result;
}

std::vector<Stop*> TransportCatalogue::GetAllStopsWithBus() {
	std::vector<Stop*> result;
	for (const auto& stops : busname_to_stop_) {
		if (stops.second.size() == 0) {
			continue;
		}
		result.push_back(stops.first);
	}
	return result;
}

void TransportCatalogue::SetBusVelocityAndWaitTime(std::pair<double, double> vel_wait) {
	bus_velocity_ = vel_wait.first;
	bus_wait_time_ = vel_wait.second;
}

double TransportCatalogue::GetBusVelocity() {
	return bus_velocity_;
}
double TransportCatalogue::GetBusWaitTime() {
	return bus_wait_time_;
}

const std::deque<Stop>& TransportCatalogue::GetAllStops() {
    return stops_;
}

const std::unordered_map<std::pair<Stop*, Stop*>, double, PairStopHasher>& TransportCatalogue::GetAllDistances() {
    return distances_;
}
