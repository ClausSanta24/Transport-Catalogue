#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader(std::istream& cin) {
	MakeRequests(cin);
}

TransportCatalogue JsonReader::MakeCatalogue(const json::Node& base_reqs) {
	TransportCatalogue result;
	std::map<std::string, std::map<std::string, double>> distances;

	for (const auto& request : base_reqs.AsArray()) {
		if (request.AsDict().at("type"s) == "Stop"s) {
			result.AddStop(MakeStop(request, distances));
		}
	}

	for (const auto& [left, right] : distances) {
		for (const auto& [name, dist] : right) {
			result.AddDistance(&result.FindStop(left), &result.FindStop(name), dist);
		}
	}

	for (const auto& request : base_reqs.AsArray()) {
		if (request.AsDict().at("type"s) == "Bus"s) {
			result.AddBus(MakeBus(result, request));
		}
	}

    if (is_routing_settings_) {
        double bus_velocity = 0;
        double bus_wait_time = 0;
        for (const auto& [name, value] : routing_settings_.AsDict()) {
            if (name == "bus_velocity"s) {
                bus_velocity = value.AsDouble();
            }
            if (name == "bus_wait_time"s) {
                bus_wait_time = value.AsDouble();
            }
        }
        result.SetBusVelocityAndWaitTime(std::make_pair(bus_velocity, bus_wait_time));
    }

	return result;
}

json::Node& JsonReader::GetBaseReq() {
	return base_reqs_;
}

json::Node& JsonReader::GetStatReq() {
	return stat_reqs_;
}

json::Node& JsonReader::GetRenderSettings() {
	return render_settings_;
}

Stop JsonReader::MakeStop(const json::Node& request, std::map<std::string, std::map<std::string, double>>& distances) {
	Stop tmp;
	for (const auto& [key, value] : request.AsDict()) {
		if (key == "type"s) {
			continue;
		}
		if (key == "name"s) {
			tmp.name = value.AsString();
		}
		if (key == "latitude"s) {
			tmp.lat = value.AsDouble();
		}
		if (key == "longitude"s) {
			tmp.lng = value.AsDouble();
		}
		if (key == "road_distances"s) {
			for (const auto& [name, dist] : value.AsDict()) {
				distances[tmp.name][name] += dist.AsDouble();
			}
		}
	}
	return tmp;
}

Bus JsonReader::MakeBus(TransportCatalogue& catalogue, const json::Node& request) {
	Bus res;
	for (const auto& [key, value] : request.AsDict()) {
		if (key == "type"s) {
			continue;
		}
		if (key == "name"s) {
			res.name = value.AsString();
		}
		if (key == "is_roundtrip"s) {
			res.is_round = value.AsBool();
		}
		if (key == "stops"s) {
			for (const auto& stop : value.AsArray()) {
				res.bus.push_back(&catalogue.FindStop(stop.AsString()));
			}
		}
	}
	double straight_distance = 0;
	double right_distance = 0;
	bool is_first = true;
	for (size_t i = 0; i < res.bus.size(); ++i) {
		if (is_first) {
			is_first = false;
			continue;
		}
		Coordinates left;
		Coordinates right;
		left.lat = res.bus[i - 1]->lat;
		left.lng = res.bus[i - 1]->lng;
		right.lat = res.bus[i]->lat;
		right.lng = res.bus[i]->lng;
		straight_distance += ComputeDistance(left, right);
		double temp_dist = catalogue.GetDistance(res.bus[i - 1], res.bus[i]);
		right_distance += temp_dist == 0 ? catalogue.GetDistance(res.bus[i], res.bus[i - 1]) : temp_dist;
	}
	if (!res.is_round) {
		straight_distance *= 2;
		for (auto i = res.bus.size() - 1; i > 0; --i) {
			double tmp = catalogue.GetDistance(res.bus[i], res.bus[i - 1]);
			right_distance += tmp == 0 ? catalogue.GetDistance(res.bus[i - 1], res.bus[i]) : tmp;
		}
	}
	res.lenght = right_distance;
	res.curvature = right_distance / straight_distance;
	return res;
}

void JsonReader::MakeRequests(std::istream& cin) {
	json::Document json = json::Load(cin);
	for (const auto& [name, req] : json.GetRoot().AsDict()) {
		if (name == "base_requests") {
			base_reqs_ = std::move(req);
			continue;
		}
		if (name == "stat_requests") {
			stat_reqs_ = std::move(req);
			continue;
		}
		if (name == "render_settings") {
			render_settings_ = std::move(req);
            is_render_settings_ = true;
			continue;
		}
		if (name == "routing_settings") {
			routing_settings_ = std::move(req);
            is_routing_settings_ = true;
			continue;
		}
        if (name == "serialization_settings") {
            //serialization_settings_ = req.AsString();
            serialization_settings_ = req.AsDict().at("file").AsString();
            continue;
        }
	}
}

std::string JsonReader::GetSerializationSettings() {
    return serialization_settings_;
}
