#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <algorithm>
#include <optional>
#include <variant>
#include <iomanip>      

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace svg {

using namespace std::literals;

inline const std::string NoneColor{ "none" };

struct Rgb {

    Rgb(){}
    Rgb(unsigned int red_enter, unsigned int green_enter, unsigned int blue_enter) :
         red(red_enter), green(green_enter), blue(blue_enter) { }

    uint8_t red = 0, green = 0, blue = 0;
};

struct Rgba {

    Rgba() {}
    Rgba(unsigned int red_enter, unsigned int green_enter, unsigned int blue_enter, double opacity_enter) :
        red(red_enter), green(green_enter), blue(blue_enter), opacity(opacity_enter){ }

    uint8_t red = 0, green = 0, blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << NoneColor;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb color) const {
        out << "rgb("sv << static_cast<uint16_t>(color.red) << ","sv << static_cast<uint16_t>(color.green) << ","sv << static_cast<uint16_t>(color.blue) << ")"sv;
    }
    void operator()(Rgba color) const {
        out << "rgba("sv << static_cast<uint16_t>(color.red) << ","sv << static_cast<uint16_t>(color.green) << ","sv << static_cast<uint16_t>(color.blue) << ","sv << std::setprecision(6) <<color.opacity << ")"sv;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Color& data)
{
    std::visit(ColorPrinter{ os }, data);
    return os;
}

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

inline std::ostream& operator<<(std::ostream& os, const StrokeLineCap& data)
{
    switch (data)
    {
    case StrokeLineCap::BUTT:
        os << "butt"s;
        break;
    case StrokeLineCap::ROUND:
        os << "round"s;
        break;
    case StrokeLineCap::SQUARE:
        os << "square"s;
        break;
    default:
        break;
    }
    return os;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& data)
{
    switch (data)
    {
    case StrokeLineJoin::ARCS:
        os << "arcs"s;
        break;
    case StrokeLineJoin::BEVEL:
        os << "bevel"s;
        break;
    case StrokeLineJoin::MITER:
        os << "miter"s;
        break;
    case StrokeLineJoin::MITER_CLIP:
        os << "miter-clip"s;
        break;
    case StrokeLineJoin::ROUND:
        os << "round"s;
        break;
    default:
        break;
    }
    return os;
}

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (width_) {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }

    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;

};


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:

    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:

    Polyline() = default;

    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);
    size_t SizePoints() const;

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
    
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point position_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;

};

class ObjectContainer {
public:
    ObjectContainer() = default;
    
    virtual ~ObjectContainer() = default;

    template<typename Object_type>
    void Add(Object_type object) {
        AddPtr(std::make_unique<Object_type>(object));
    }
    
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::vector<std::unique_ptr<Object>> objects_;

};

class Document final : public ObjectContainer {
public:
 
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

};

class Drawable {
public:
    Drawable() = default;
    virtual ~Drawable() = default;

    virtual void Draw(ObjectContainer& container) const = 0;

};

}  // namespace svg

namespace shapes {
    using namespace std::literals;
    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
            : p1_(p1)
            , p2_(p2)
            , p3_(p3) {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
        }

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public svg::Drawable {
    public:

        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) :
            center_ (center), outer_rad_(outer_rad), 
            inner_rad_(inner_rad), num_rays_(num_rays) {
        }

        svg::Polyline CreateStar(const svg::Point center, const double outer_rad, const double inner_rad, const int num_rays) const {
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
            polyline.SetFillColor("red"s);
            polyline.SetStrokeColor("black"s);
            return polyline;
        }

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_));
        }

    private:
        svg::Point center_;
        double outer_rad_, inner_rad_;
        int num_rays_;
    };

    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point center, double radius) :center_(center), radius_(radius) {

        }

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Circle().SetCenter({ center_.x, center_.y + radius_ * 5 }).SetRadius(radius_ * 2).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
            container.Add(svg::Circle().SetCenter({ center_.x, center_.y + radius_ * 2}).SetRadius(radius_ * 1.5).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
            container.Add(svg::Circle().SetCenter(center_).SetRadius(radius_).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
        }

    private:
        svg::Point center_;
        double radius_ = 1.0;
    };

} // namespace shapes 