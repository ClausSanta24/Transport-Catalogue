#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "json.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

using namespace domain;

inline const double EPSILON = 1e-6;
bool IsZero(double value);
namespace renderer {


class SphereProjector {
public:

    SphereProjector() {}
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct Settings {

    Settings(const json::Node& node);
    Settings() {}

    double width, height, padding, stop_radius, line_width, underlayer_width;
    int bus_label_font_size, stop_label_font_size;
    std::vector<double> bus_label_offset, stop_label_offset;
    svg::Color underlayer_color;
    std::vector<svg::Color> color_palette;

    svg::Color WhatAColor(const json::Node& value);
};

class MapRenderer {

public:

    MapRenderer() {}

    MapRenderer(const Settings& s);

    void AddCoord(geo::Coordinates coords);
    const std::vector<svg::Point>& GetPointsByBus(Bus& bus);
    void MakeSphereProj();
    void AddPoints(Bus& bus);
    Settings& GetSettings();
    SphereProjector& GetProj();
    svg::Polyline MakePolylines(const std::vector<svg::Point>& points_on_screen, int counter) const;
    std::vector<svg::Text> MakeBusNames(const std::vector<svg::Point>& points_on_screen, const std::string& name, int counter, const Bus* bus) const;
    std::vector<svg::Circle> MakeCircles(std::map<std::string, svg::Point>& stops) const;
    std::vector<svg::Text> MakeStopNames(std::map<std::string, svg::Point>& stops) const;
    svg::Document MakeSvgDoc(const std::vector<Bus*> buses, const std::vector<Stop*> stops);
private:

    Settings settings_;
    SphereProjector proj_;
    std::vector<geo::Coordinates> geo_coords_;
    std::map<std::string, std::vector<svg::Point>> points_;
    double width_;
    double height_;
    double padding_;
};

/*
int main() {
    using namespace std;

    const double WIDTH = 600.0;
    const double HEIGHT = 400.0;
    const double PADDING = 50.0;

    // Точки, подлежащие проецированию
    vector<geo::Coordinates> geo_coords = {
        {43.587795, 39.716901}, {43.581969, 39.719848}, {43.598701, 39.730623},
        {43.585586, 39.733879}, {43.590317, 39.746833}
    };

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), WIDTH, HEIGHT, PADDING
    };

    // Проецируем и выводим координаты
    for (const auto geo_coord : geo_coords) {
        const svg::Point screen_coord = proj(geo_coord);
        cout << '(' << geo_coord.lat << ", "sv << geo_coord.lng << ") -> "sv;
        cout << '(' << screen_coord.x << ", "sv << screen_coord.y << ')' << endl;
    }
}
*/

} // namespace renderer