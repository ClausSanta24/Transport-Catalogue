#include <fstream>
#include <iostream>
#include <string_view>

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <transport_catalogue.pb.h>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        // make base here
        JsonReader json(std::cin);
        TransportCatalogue catalogue = json.MakeCatalogue(json.GetBaseReq());
        renderer::Settings settings(json.GetRenderSettings());
        std::ofstream out(json.GetSerializationSettings(), std::ios::binary);
        SerializeAll(SerializeCatalogue(catalogue), SerializeSettings(settings), out);
        //SerializeCatalogue(catalogue);

    } else if (mode == "process_requests"sv) {

        // process requests here
        JsonReader json(std::cin);
        std::ifstream in(json.GetSerializationSettings(), std::ios::binary);
        TransportCatalogue catalogue;
        renderer::Settings settings;
        transport_catalogue_proto::ProtoStruct proto_struct = DeserializeAll(in);
        DeserializeCatalogue(proto_struct.catalogue(), catalogue);
        settings = DeserializeSettings(proto_struct.settings());
        renderer::MapRenderer rend(settings);
        RequestHandler req(catalogue, rend);
        auto a = catalogue.GetAllBuses();
        auto b = catalogue.GetAllDistances();
        auto c = catalogue.GetAllStops();
        auto d = catalogue.GetAllStopsWithBus();
        auto e = catalogue.GetBusesByStop("9");
        req.CatalogueRequest(json.GetStatReq(), std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
