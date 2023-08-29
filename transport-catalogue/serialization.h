#pragma once

#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>

#include <iostream>
#include <string>
#include <deque>
#include <vector>

using namespace renderer;
using namespace svg;

template <typename it>
uint32_t CalculateID(it begin, it end, std::string name) {
    it result = std::find_if(begin, end,
                           [&name]
                           (const Stop stop)
                           {
                               return stop.name == name;
                           }
    );
    return std::distance(begin, result);
}

transport_catalogue_proto::TransportCatalogue SerializeCatalogue(TransportCatalogue& catalogue);


void DeserializeCatalogue(transport_catalogue_proto::TransportCatalogue catalogue, TransportCatalogue& result);

transport_catalogue_proto::Color SerializeColor(const Color& color);

Color DeserializeColor(const transport_catalogue_proto::Color& color);

transport_catalogue_proto::RenderSettings SerializeSettings(const Settings& renderer);

Settings DeserializeSettings(const transport_catalogue_proto::RenderSettings& settings);

void SerializeAll(transport_catalogue_proto::TransportCatalogue tc, transport_catalogue_proto::RenderSettings rs, std::ostream& out);

transport_catalogue_proto::ProtoStruct DeserializeAll(std::istream& in);
