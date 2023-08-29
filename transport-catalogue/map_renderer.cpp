#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
namespace renderer {

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    Settings::Settings(const json::Node& node) {
        for (const auto& [name, value] : node.AsDict()) {
            if (name == "width"s) {
                width = value.AsDouble();
            }
            if (name == "height"s) {
                height = value.AsDouble();
            }
            if (name == "padding"s) {
                padding = value.AsDouble();
            }
            if (name == "stop_radius"s) {
                stop_radius = value.AsDouble();
            }
            if (name == "line_width"s) {
                line_width = value.AsDouble();
            }
            if (name == "bus_label_font_size"s) {
                bus_label_font_size = value.AsInt();
            }
            if (name == "bus_label_offset"s) {
                for (const auto& val : value.AsArray()) {
                    bus_label_offset.push_back(val.AsDouble());
                }
            }
            if (name == "stop_label_font_size"s) {
                stop_label_font_size = value.AsInt();
            }
            if (name == "stop_label_offset"s) {
                for (const auto& val : value.AsArray()) {
                    stop_label_offset.push_back(val.AsDouble());
                }
            }
            if (name == "underlayer_color"s) {
                underlayer_color = WhatAColor(value);
            }
            if (name == "underlayer_width"s) {
                underlayer_width = value.AsDouble();
            }
            if (name == "color_palette"s) {
                for (const auto& a : value.AsArray()) {
                    color_palette.push_back(WhatAColor(a));
                }
            }
        }


        /*
        "width": 600,
        "height": 400,
        "padding": 50,
        "stop_radius": 5,
        "line_width": 14,
        "bus_label_font_size": 20,
        "bus_label_offset": [
          7,
          15
        ],
        "stop_label_font_size": 20,
        "stop_label_offset": [
          7,
          -3
        ],
        "underlayer_color": [
          255,
          255,
          255,
          0.85
        ],
        "underlayer_width": 3,
        "color_palette": [
          "green",
          [255, 160, 0],
          "red"
        ]
        */

        /*double width, height, padding, stop_radius, line_width;
        int bus_label_font_size, stop_label_font_size, underlayer_width;
        std::vector<double> bus_label_offset, stop_label_offset;
        svg::Color underlayer_color;
        std::vector<svg::Color> color_palette;*/
    }

    svg::Color Settings::WhatAColor(const json::Node& value) {
        if (value.IsString()) {
            return value.AsString();
        }
        else if (value.AsArray().size() == 3) {
            uint8_t r, g, b;
            r = static_cast<uint8_t>(value.AsArray()[0].AsInt());
            g = static_cast<uint8_t>(value.AsArray()[1].AsInt());
            b = static_cast<uint8_t>(value.AsArray()[2].AsInt());
            return svg::Rgb(r, g, b);
        }
        else {
            uint8_t r, g, b;
            double o;
            r = static_cast<uint8_t>(value.AsArray()[0].AsInt());
            g = static_cast<uint8_t>(value.AsArray()[1].AsInt());
            b = static_cast<uint8_t>(value.AsArray()[2].AsInt());
            o = value.AsArray()[3].AsDouble();
            return svg::Rgba(r, g, b, o);
        }
    }

    MapRenderer::MapRenderer(const Settings& s)
        : settings_(s)
    {
        width_ = s.width;
        height_ = s.height;
        padding_ = s.padding;

    }

    void MapRenderer::AddCoord(geo::Coordinates coords) {
        geo_coords_.push_back(coords);
    }

    const std::vector<svg::Point>& MapRenderer::GetPointsByBus(Bus& bus) {
        return points_[bus.name];
    }

    void MapRenderer::MakeSphereProj() {
        proj_ = SphereProjector{ geo_coords_.begin(), geo_coords_.end(), width_, height_, padding_ };
    }

    void MapRenderer::AddPoints(Bus& bus) {
        for (const auto& a : bus.bus) {
            points_[bus.name].push_back(proj_({ a->lat, a->lng }));
        }
        if (!bus.is_round) {
            // points_[bus.name].push_back(proj_({ bus.bus.front()->lat, bus.bus.front()->lng }));
            // return;
            for (auto i = bus.bus.size() - 1; i > 0; --i) {
                points_[bus.name].push_back(proj_({ bus.bus[i - 1]->lat, bus.bus[i - 1]->lng }));
            }
            //std::vector<svg::Point> tmp;
            //std::copy(points_[bus.name].end() - 2, points_[bus.name].begin(), tmp.begin());
            //points_[bus.name].insert(points_[bus.name].end(), tmp.begin(), tmp.end());
        }


    }

    Settings& MapRenderer::GetSettings() {
        return settings_;
    }

    SphereProjector& MapRenderer::GetProj() {
        return proj_;
    }

    svg::Polyline MapRenderer::MakePolylines(const std::vector<svg::Point>& points_on_screen, int counter) const {
        svg::Polyline tmp;
        for (const auto& point : points_on_screen) {
            tmp.AddPoint(point);
        }
        tmp.SetFillColor(svg::Color{});
        tmp.SetStrokeColor(settings_.color_palette[counter]);
        tmp.SetStrokeWidth(settings_.line_width);
        tmp.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        tmp.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        return tmp;
    }

    std::vector<svg::Text> MapRenderer::MakeBusNames(const std::vector<svg::Point>& points_on_screen, const std::string& name, int counter, const Bus* bus) const {
        std::vector<svg::Text> result;
        svg::Text tmp;
        svg::Text tmp1;
        tmp.SetPosition(points_on_screen.front());
        tmp.SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] });
        tmp.SetFontSize(settings_.bus_label_font_size);
        tmp.SetFontFamily("Verdana"s);
        tmp.SetFontWeight("bold"s);
        tmp.SetData(name);
        tmp1 = tmp;
        tmp.SetFillColor(settings_.color_palette[counter]);
        tmp1.SetFillColor(settings_.underlayer_color);
        tmp1.SetStrokeColor(settings_.underlayer_color);
        tmp1.SetStrokeWidth(settings_.underlayer_width);
        tmp1.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        tmp1.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        result.push_back(tmp1);
        result.push_back(tmp);
        if (!(bus->is_round) && bus->bus.front() != bus->bus.back()) {
            svg::Point back = proj_({ bus->bus.back()->lat, bus->bus.back()->lng });
            tmp.SetPosition(back);
            tmp1.SetPosition(back);
            result.push_back(tmp1);
            result.push_back(tmp);
        }
        return result;
    }

    std::vector<svg::Circle> MapRenderer::MakeCircles(std::map<std::string, svg::Point>& stops) const {
        std::vector<svg::Circle> result;
        result.reserve(stops.size());

        for (const auto& a : stops) {
            svg::Circle tmp;
            tmp.SetCenter(a.second);
            tmp.SetRadius(settings_.stop_radius);
            tmp.SetFillColor("white"s);
            result.push_back(tmp);
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::MakeStopNames(std::map<std::string, svg::Point>& stops) const {
        std::vector<svg::Text> result;
        for (const auto& a : stops) {
            svg::Text tmp;
            svg::Text tmp1;
            tmp.SetPosition(a.second);
            tmp.SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] });
            tmp.SetFontSize(settings_.stop_label_font_size);
            tmp.SetFontFamily("Verdana"s);
            tmp.SetData(a.first);
            tmp1 = tmp;
            tmp.SetFillColor("black"s);
            tmp1.SetFillColor(settings_.underlayer_color);
            tmp1.SetStrokeColor(settings_.underlayer_color);
            tmp1.SetStrokeWidth(settings_.underlayer_width);
            tmp1.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            tmp1.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            result.push_back(tmp1);
            result.push_back(tmp);
        }
        return result;
    }

    svg::Document MapRenderer::MakeSvgDoc(const std::vector<Bus*> buses, const std::vector<Stop*> stops) {
        svg::Document result;
        for (const auto& bus : buses) {
            for (const auto& stop : bus->bus) {
                AddCoord({ stop->lat, stop->lng });
            }
        }
        MakeSphereProj();
        std::map<std::string, std::vector<svg::Point>> points_on_screen;
        for (const auto& bus : buses) {
            AddPoints(*bus);
            points_on_screen[bus->name] = GetPointsByBus(*bus);
        }

        size_t i = 0;

        for (const auto& a : points_on_screen) {
            if (i == GetSettings().color_palette.size()) {
                i = 0;
            }
            result.Add(MakePolylines(a.second, i));
            ++i;
        }
        i = 0;


        for (const auto& a : points_on_screen) {
            if (i == GetSettings().color_palette.size()) {
                i = 0;
            }
            const auto busik = std::find_if(buses.begin(), buses.end(), [a](const Bus* c) {return c->name == a.first; });
            for (const auto& b : MakeBusNames(a.second, a.first, i, *busik)) {
                result.Add(b);
            }
            ++i;
        }
        std::map<std::string, svg::Point> stops_;

        for (const auto& stop : stops) {
            stops_[stop->name] = proj_({ stop->lat, stop->lng });
        }

        for (const auto& circle : MakeCircles(stops_)) {
            result.Add(circle);
        }

        for (const auto& stop : MakeStopNames(stops_)) {
            result.Add(stop);
        }

        //result.Render(std::cout);

        return result;
        //return svg::Document{};
    }

}//namespace renderer