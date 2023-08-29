#include "svg.h"

namespace svg {

    using namespace std::literals;

    bool operator==(const Point& l, const Point& r) {
        return l.x == r.x && l.y == r.y;
    }
    bool operator!=(const Point& l, const Point& r) {
        return !(l == r);
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polilyne ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool is_first = true;
        for (const auto& point : points_) {
            if (is_first) {
                out << point.x << ","sv << point.y;
                is_first = false;
            }
            else {
                out << " "sv << point.x << ","sv << point.y;
            }
        }

        out << "\"";
        RenderAttrs(context.out);
        out << "/>";
    }

    // ---------- Text ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        std::string result;
        for (const auto& c : data) {
            switch (c)
            {
            case '"':
                result += "&quot;"sv;
                break;
            case '<':
                result += "&lt;"sv;
                break;
            case '>':
                result += "&gt;"sv;
                break;
            case '\'':
                result += "&apos;"sv;
                break;
            case '&':
                result += "&amp;"sv;
                break;
            default:
                result.push_back(c);
                break;
            }
        }
        data_ = result;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        if (font_family_ != ""sv) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_ != ""sv) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << " font-size=\""sv << size_ << "\""sv;
        out << " x=\""sv << pos_.x << "\""sv;
        out << " y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\""sv;
        out << " dy=\""sv << offset_.y << "\""sv;
        RenderAttrs(context.out);
        out << " >" << data_ << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostringstream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto& object : objects_) {
            object.get()->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg

namespace shapes {
    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        using namespace svg;
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }
        return polyline;
    }
}