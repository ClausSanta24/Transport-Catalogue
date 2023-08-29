#pragma once
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <deque>
#include <string_view>
#include <iostream>
#include <algorithm>
#include <set>

#include "geo.h"
#include "domain.h"

using namespace domain;
using namespace geo;

const size_t HASHER_NUMBER = 37;

struct PairStopHasher {
    size_t operator()(const std::pair<Stop*, Stop*> p) const {
        const size_t p1 = reinterpret_cast<size_t>(p.first);
        const size_t p2 = reinterpret_cast<size_t>(p.second);
        return p1 * HASHER_NUMBER + p2 * HASHER_NUMBER * HASHER_NUMBER;
    }
};


class TransportCatalogue {
public:

	TransportCatalogue() {}
	~TransportCatalogue() {}

	void AddStop(const Stop& stop);
	Stop& FindStop(const std::string& stop);
	void AddBus(const Bus& bus);
	Bus& FindBus(const std::string& bus);
	Bus& GetBusInfo(const std::string& bus) ;
	std::vector<Bus*> GetBusesByStop(const std::string& stop) ;
	void AddDistance(Stop* left, Stop* right, double distance);
	double GetDistance(Stop* left, Stop* right);
	std::vector<Bus*> GetAllBuses();
	std::vector<Stop*> GetAllStopsWithBus();
	void SetBusVelocityAndWaitTime(std::pair<double,double> vel_wait);
	double GetBusVelocity();
	double GetBusWaitTime();
    const std::deque<Stop>& GetAllStops();
    const std::unordered_map<std::pair<Stop*, Stop*>, double, PairStopHasher>& GetAllDistances();


private:


	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Bus*> busname_to_bus_;
	std::unordered_map<std::pair<Stop*, Stop*>, double, PairStopHasher> distances_;
	std::unordered_map<Stop*, std::vector<Bus*>> busname_to_stop_;
	double bus_velocity_, bus_wait_time_;

};
