#pragma once

#include <iostream>
#include <string>
#include <set>
#include <iomanip>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "json_builder.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"

using namespace std::literals;
using namespace domain;

inline renderer::MapRenderer EMPTY_RENDERER{};

class RequestHandler {
public:

    void PrintBusInfo(const std::string& name, std::ostream& cout, int id);
    void PrintStopInfo(const std::string& name, std::ostream& cout, int id);
    void PrintRouteInfo(const std::string& from, const std::string& to, int id, std::ostream& cout);
    void CatalogueRequest(json::Node& json, std::ostream& cout);


    // MapRenderer понадобится в следующей части итогового проекта
    
    RequestHandler(TransportCatalogue& db, renderer::MapRenderer& renderer = EMPTY_RENDERER);


    // Возвращает информацию о маршруте (запрос Bus)
    Bus& GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    std::vector<Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    // Возвращает все маршруты
    std::vector<Bus*> GetAllBuses() const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    TransportCatalogue& db_;
    TransportRouter<double> router_;
    renderer::MapRenderer& renderer_;

};


