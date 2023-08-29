#include "request_handler.h"

void RequestHandler::PrintBusInfo(const std::string& name, std::ostream& cout, int id) {
	const auto& bus = db_.GetBusInfo(name);
	if (bus.name == EMPTY_BUS.name) {
		json::Print(json::Document{ json::Dict{
			{"request_id"s, id},
			{"error_message"s, "not found"s}
			} }, cout);
		return;
	}
	size_t stops_count = bus.bus.size();
	int unique_stops_count = 0;
	std::set<std::string> unique_stops;
	for (size_t i = 0; i < stops_count; ++i) {
		if (unique_stops.count(bus.bus[i]->name) == 0) {
			unique_stops.insert(bus.bus[i]->name);
			++unique_stops_count;
		}
	}
	if (!bus.is_round) {
		stops_count = stops_count * 2 - 1;
	}
	json::Print(json::Document{ json::Dict{
		{"curvature"s, bus.curvature},
		{"request_id"s, id},
		{"route_length"s, bus.lenght},
		{"stop_count"s, static_cast<int>(stops_count)},
		{"unique_stop_count"s, unique_stops_count}
		} }, cout);
}

void RequestHandler::PrintStopInfo(const std::string& name, std::ostream& cout, int id) {
	if (db_.FindStop(name).name == EMPTY_STOP.name) {
		json::Print(json::Document{ json::Dict{
			{"request_id"s, id},
			{ "error_message"s, "not found"s }
			} }, cout);
		return;
	}
	auto buses = db_.GetBusesByStop(name);
    std::sort(buses.begin(), buses.end(),
		[](Bus* l, Bus* r)
		{
			return l->name < r->name;
        });
	json::Array buses_arr;
	for (const auto& bus : buses) {
		buses_arr.emplace_back(bus->name);
	}
	json::Print(json::Document{ json::Dict{
		{"buses"s, buses_arr},
		{"request_id"s, id}
		} }, cout);
}

void RequestHandler::PrintRouteInfo(const std::string& from, const std::string& to, int id, std::ostream& cout) {
	auto from_ = db_.FindStop(from);
	auto to_ = db_.FindStop(to);
	router_.BuildRoute(from_, to_, id, cout);
}

void RequestHandler::CatalogueRequest(json::Node& json, std::ostream& cout) {
	cout << "[\n"s;
	bool is_first = true;
	for (const auto& request : json.AsArray()) {
		if (is_first) {
			if (request.AsDict().at("type"s) == "Stop"s) {
				PrintStopInfo(request.AsDict().at("name"s).AsString(), cout, request.AsDict().at("id"s).AsInt());
			}
			if (request.AsDict().at("type"s) == "Bus"s) {
				PrintBusInfo(request.AsDict().at("name"s).AsString(), cout, request.AsDict().at("id"s).AsInt());
			}
			if (request.AsDict().at("type"s) == "Map"s) {
				svg::Document doc{ RenderMap() };
				std::ostringstream a;
				doc.Render(a);
				json::Print(json::Document{
					json::Builder{}
						.StartDict()
							.Key("map"s)
							.Value(a.str())
							.Key("request_id"s)
							.Value(request.AsDict().at("id"s).AsInt())
						.EndDict()
					.Build() }, cout);
			}
			if (request.AsDict().at("type"s) == "Route"s) {
				PrintRouteInfo(request.AsDict().at("from"s).AsString(), request.AsDict().at("to"s).AsString(), request.AsDict().at("id"s).AsInt(), cout);
			}
			is_first = false;
			continue;
		}
		cout << ",\n"s;
		if (request.AsDict().at("type"s) == "Stop"s) {
			PrintStopInfo(request.AsDict().at("name"s).AsString(), cout, request.AsDict().at("id"s).AsInt());
		}
		if (request.AsDict().at("type"s) == "Bus"s) {
			PrintBusInfo(request.AsDict().at("name"s).AsString(), cout, request.AsDict().at("id"s).AsInt());
		}
		if (request.AsDict().at("type"s) == "Map"s) {
			svg::Document doc{ RenderMap() };
			std::ostringstream a;
			doc.Render(a);
			json::Print(json::Document{
					json::Builder{}
						.StartDict()
							.Key("map"s)
							.Value(a.str())
							.Key("request_id"s)
							.Value(request.AsDict().at("id"s).AsInt())
						.EndDict()
					.Build() }, cout);
		}
		if (request.AsDict().at("type"s) == "Route"s) {
			PrintRouteInfo(request.AsDict().at("from"s).AsString(), request.AsDict().at("to"s).AsString(), request.AsDict().at("id"s).AsInt(), cout);
		}
	}
	cout << "\n]"s;
}

RequestHandler::RequestHandler(TransportCatalogue& db, renderer::MapRenderer& renderer)
    : db_(db), router_(db_), renderer_(renderer)
{
    router_.MakeGraph();
}

// Возвращает информацию о маршруте (запрос Bus)
Bus& RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return const_cast<TransportCatalogue&>(db_).GetBusInfo({ bus_name.begin(), bus_name.end() });
}

// Возвращает маршруты, проходящие через
std::vector<Bus*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	return const_cast<TransportCatalogue&>(db_).GetBusesByStop({ stop_name.begin(), stop_name.end() });
}

std::vector<Bus*> RequestHandler::GetAllBuses() const {
	return const_cast<TransportCatalogue&>(db_).GetAllBuses();
}

// Этот метод будет нужен в следующей части итогового проекта
svg::Document RequestHandler::RenderMap() const {

	svg::Document result;
	const auto buses = GetAllBuses();
	auto vec = const_cast<TransportCatalogue&>(db_).GetAllStopsWithBus();
	result = renderer_.MakeSvgDoc(buses, vec);

	//result.Render(std::cout);

	return result;
	//return svg::Document{};
}
