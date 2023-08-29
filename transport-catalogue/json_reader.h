#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */



#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <map>
#include <utility>

#include "transport_catalogue.h"
#include "json.h"
#include "domain.h"

using namespace domain;
using namespace std::literals;

class JsonReader {
public:
	JsonReader() {};
	JsonReader(std::istream& cin);
	
	TransportCatalogue MakeCatalogue(const json::Node& base_reqs);
	json::Node& GetBaseReq();
	json::Node& GetStatReq();
	json::Node& GetRenderSettings();
	
	Stop MakeStop(const json::Node& request, std::map<std::string, std::map<std::string, double>>& distances);

	Bus MakeBus(TransportCatalogue& catalogue, const json::Node& request);

    std::string GetSerializationSettings();

private:
	void MakeRequests(std::istream& cin);
	json::Node base_reqs_;
	json::Node stat_reqs_;
	json::Node render_settings_;
	json::Node routing_settings_;
    std::string serialization_settings_;
    bool is_render_settings_ = false;
    bool is_routing_settings_ = false;
};
