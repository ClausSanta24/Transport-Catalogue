#include "serialization.h"

transport_catalogue_proto::TransportCatalogue SerializeCatalogue(TransportCatalogue& catalogue) {
    transport_catalogue_proto::TransportCatalogue result;

    const auto stops = catalogue.GetAllStops();

    for (auto i = 0; i < stops.size(); ++i) {
        transport_catalogue_proto::Stop stop;
        stop.set_name(stops[i].name);
        stop.set_lat(stops[i].lat);
        stop.set_lng(stops[i].lng);
        result.mutable_stops()->Add(std::move(stop));
    }

    const auto buses = catalogue.GetAllBuses();

    for (auto i = 0; i < buses.size(); ++i) {
        transport_catalogue_proto::Bus bus;
        bus.set_name(buses[i]->name);
        bus.set_curvature(buses[i]->curvature);
        bus.set_is_round(buses[i]->is_round);
        bus.set_lenght(buses[i]->lenght);
        for (const auto& stop : buses[i]->bus) {
            bus.mutable_stops()->Add(std::move(CalculateID(stops.begin(), stops.end(), stop->name)));
        }
        result.mutable_buses()->Add(std::move(bus));
    }

    const auto distances = catalogue.GetAllDistances();

    for (const auto& [first, second] : distances) {
        transport_catalogue_proto::Distance distance;
        distance.set_start(CalculateID(stops.begin(), stops.end(), first.first->name));
        distance.set_end(CalculateID(stops.begin(), stops.end(), first.second->name));
        distance.set_distance(second);
        result.mutable_distances()->Add(std::move(distance));
    }

    result.set_bus_velocity(catalogue.GetBusVelocity());
    result.set_bus_wait_time(catalogue.GetBusWaitTime());

    //result.SerializePartialToOstream(&out);
    return result;
}

void DeserializeCatalogue(transport_catalogue_proto::TransportCatalogue catalogue, TransportCatalogue& result) {

    const auto stops = catalogue.stops();

    for (auto i = 0; i < stops.size(); ++i) {
        Stop stop;
        stop.lat = stops[i].lat();
        stop.lng = stops[i].lng();
        stop.name = stops[i].name();
        result.AddStop(std::move(stop));
    }

    const auto buses = catalogue.buses();

    for (auto i = 0; i < buses.size(); ++ i) {
        Bus bus;
        bus.name = buses[i].name();
        bus.lenght = buses[i].lenght();
        bus.is_round = buses[i].is_round();
        bus.curvature = buses[i].curvature();
        for (const auto& id : buses[i].stops()) {
            bus.bus.push_back(&result.FindStop(stops[id].name()));
        }
        result.AddBus(std::move(bus));
    }

    const auto distances = catalogue.distances();

    for (auto i = 0; i < distances.size(); ++i) {
        Stop* left = &result.FindStop(stops[distances[i].start()].name());
        Stop* right = &result.FindStop(stops[distances[i].end()].name());
        result.AddDistance(left, right, distances[i].distance());
    }

    /*    result.set_bus_velocity(catalogue.GetBusVelocity());
    result.set_bus_wait_time(catalogue.GetBusWaitTime());*/

    //void SetBusVelocityAndWaitTime(std::pair<double,double> vel_wait);
    result.SetBusVelocityAndWaitTime(std::make_pair<double, double>(catalogue.bus_velocity(), catalogue.bus_wait_time()));

}

transport_catalogue_proto::Color SerializeColor(const Color& color){
    transport_catalogue_proto::Color result;

    if (std::holds_alternative<std::monostate>(color)) {
        result.set_empty(true);
    } else if (std::holds_alternative<std::string>(color)) {
        result.set_string_color(std::get<std::string>(color));
    } else if (std::holds_alternative<Rgb>(color)) {
        Rgb rgb = std::get<Rgb>(color);
        result.mutable_rgb()->set_r(rgb.red);
        result.mutable_rgb()->set_g(rgb.green);
        result.mutable_rgb()->set_b(rgb.blue);
    } else if (std::holds_alternative<Rgba>(color)) {
        Rgba rgba = std::get<Rgba>(color);
        result.mutable_rgba()->set_r(rgba.red);
        result.mutable_rgba()->set_g(rgba.green);
        result.mutable_rgba()->set_b(rgba.blue);
        result.mutable_rgba()->set_o(rgba.opacity);
    }
    return result;
}

Color DeserializeColor(const transport_catalogue_proto::Color& color){

    Color result;
    if (color.has_rgb()) {
        Rgb rgb;
        rgb.red = color.rgb().r();
        rgb.blue = color.rgb().b();
        rgb.green = color.rgb().g();
        result = rgb;
    } else if (color.has_rgba()) {
        Rgba rgba;
        rgba.red = color.rgba().r();
        rgba.blue = color.rgba().b();
        rgba.green = color.rgba().g();
        rgba.opacity = color.rgba().o();
        result = rgba;
    } else {
        result = color.string_color();
    }

    return result;
}

transport_catalogue_proto::RenderSettings SerializeSettings(const Settings& renderer) {

    transport_catalogue_proto::RenderSettings settings;

    settings.set_width(renderer.width);
    settings.set_height(renderer.height);
    settings.set_padding(renderer.padding);
    settings.set_stop_radius(renderer.stop_radius);
    settings.set_line_width(renderer.line_width);
    settings.set_underlayer_width(renderer.underlayer_width);
    settings.set_bus_label_font_size(renderer.bus_label_font_size);
    settings.set_stop_label_font_size(renderer.stop_label_font_size);

    for (const auto& a : renderer.bus_label_offset) {
        settings.mutable_bus_label_offset()->Add(a);
    }

    for (const auto& a : renderer.stop_label_offset) {
        settings.mutable_stop_label_offset()->Add(a);
    }

    *settings.mutable_underlayer_color() = std::move(SerializeColor(renderer.underlayer_color));

    for (const auto& a : renderer.color_palette) {
        settings.mutable_color_palette()->Add(std::move(SerializeColor(a)));
    }

    return settings;
}

Settings DeserializeSettings(const transport_catalogue_proto::RenderSettings& settings) {

    Settings result;

    result.width = settings.width();
    result.height = settings.height();
    result.padding = settings.padding();
    result.stop_radius = settings.stop_radius();
    result.line_width = settings.line_width();
    result.underlayer_width = settings.underlayer_width();
    result.bus_label_font_size = settings.bus_label_font_size();
    result.stop_label_font_size = settings.stop_label_font_size();

    for (const auto& a : settings.bus_label_offset()) {
        result.bus_label_offset.push_back(a);
    }

    for (const auto& a : settings.stop_label_offset()) {
        result.stop_label_offset.push_back(a);
    }

    result.underlayer_color = DeserializeColor(settings.underlayer_color());

    for (const auto& a : settings.color_palette()) {
        result.color_palette.push_back(DeserializeColor(a));
    }

    return result;

}

void SerializeAll(transport_catalogue_proto::TransportCatalogue tc, transport_catalogue_proto::RenderSettings rs, std::ostream& out) {
    transport_catalogue_proto::ProtoStruct result;
    *result.mutable_catalogue() = std::move(tc);
    *result.mutable_settings() = std::move(rs);
    result.SerializePartialToOstream(&out);
}

transport_catalogue_proto::ProtoStruct DeserializeAll(std::istream& in) {
    transport_catalogue_proto::ProtoStruct result;
    result.ParseFromIstream(&in);
    return result;
}
