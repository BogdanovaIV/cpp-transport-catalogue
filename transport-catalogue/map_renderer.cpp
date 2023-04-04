#include "map_renderer.h"

namespace map_renderer {
    using namespace std::literals;

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::Project(geo::Coordinates coords) const {
        return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return Project(coords);
    }

    map::map(std::unordered_map<std::string, double>& parametrs, std::pair<double, double>& bus_label_offset,
        std::pair<double, double>& stop_label_offset, svg::Color& underlayer_color, std::vector<svg::Color>& color_palette,
        std::vector<geo::Coordinates>& coordinates, std::vector<std::pair<const transport::TransportCatalogue::Stop*, const transport::TransportCatalogue::Bus*>>& Stops_Buses) :
        SphereProjector(coordinates.begin(), coordinates.end(), parametrs.at("width"), parametrs.at("height"), parametrs.at("padding")),
        width_(parametrs.at("width")),
        height_(parametrs.at("height")),
        line_width_(parametrs.at("line_width")),
        stop_radius_(parametrs.at("stop_radius")),
        bus_label_font_size_(parametrs.at("bus_label_font_size")),
        bus_label_offset_(std::move(bus_label_offset)),
        stop_label_font_size_(parametrs.at("stop_label_font_size")),
        stop_label_offset_(std::move(stop_label_offset)),
        underlayer_color_(std::move(underlayer_color)),
        underlayer_width_(parametrs.at("underlayer_width")),
        color_palette_(std::move(color_palette)),
        Stops_Buses_(Stops_Buses) {
    }

    void map::MakeTextForBus(const svg::Color& color, const svg::Point& point, const transport::TransportCatalogue::Bus* bus, std::vector<svg::Text>& text_buses) const {
        svg::Text text_background;
        text_background.SetFillColor(underlayer_color_);
        text_background.SetStrokeColor(underlayer_color_);
        text_background.SetStrokeWidth(underlayer_width_);
        text_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_background.SetPosition(point);
        text_background.SetOffset({ bus_label_offset_.first, bus_label_offset_.second });
        text_background.SetFontSize(bus_label_font_size_);
        text_background.SetFontFamily("Verdana"s);
        text_background.SetFontWeight("bold"s);
        text_background.SetData(bus->name);
        text_buses.push_back(std::move(text_background));

        svg::Text text_bus;
        text_bus.SetFillColor(color);
        text_bus.SetPosition(point);
        text_bus.SetOffset({ bus_label_offset_.first, bus_label_offset_.second });
        text_bus.SetFontSize(bus_label_font_size_);
        text_bus.SetFontFamily("Verdana"s);
        text_bus.SetFontWeight("bold"s);
        text_bus.SetData(bus->name);
        text_buses.push_back(std::move(text_bus));
    }

    void map::MakeTextForStop(const svg::Point & point, const transport::TransportCatalogue::Stop * stop, svg::Document& doc) const {
        svg::Text text_background;
        text_background.SetFillColor(underlayer_color_);
        text_background.SetStrokeColor(underlayer_color_);
        text_background.SetStrokeWidth(underlayer_width_);
        text_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_background.SetPosition(point);
        text_background.SetOffset({ stop_label_offset_.first, stop_label_offset_.second });
        text_background.SetFontSize(stop_label_font_size_);
        text_background.SetFontFamily("Verdana"s);
        text_background.SetData(stop->name);
        doc.Add(std::move(text_background));

        svg::Text text_bus;
        text_bus.SetFillColor(svg::Color{"black"s});
        text_bus.SetPosition(point);
        text_bus.SetOffset({ stop_label_offset_.first, stop_label_offset_.second });
        text_bus.SetFontSize(stop_label_font_size_);
        text_bus.SetFontFamily("Verdana"s);
        text_bus.SetData(stop->name);
        doc.Add(std::move(text_bus));
    }

    void map::Render(std::ostream& out) const {
        svg::Document doc;
        int i = -1;
        std::vector<svg::Text> text_buses;
        const transport::TransportCatalogue::Bus* bus = nullptr;
        svg::Polyline polyline;

        struct OrderingRule {
            bool operator()(const transport::TransportCatalogue::Stop* lhs, const transport::TransportCatalogue::Stop* rhs) const
            {
                return lhs->name < rhs->name;
            }
        };

        std::set<const transport::TransportCatalogue::Stop*, OrderingRule> unique_stops;
        for (auto& route : Stops_Buses_) {
            svg::Point point = Project({ route.first->latitude, route.first->longitude });
            if (route.second != bus) {
                if (bus != nullptr) {
                    doc.Add(std::move(polyline));
                    polyline = svg::Polyline{};
                    if (!bus->is_roundtrip && bus->stop_begin != bus->stop_end) {
                        MakeTextForBus(color_palette_.at(i), Project({ bus->stop_end->latitude, bus->stop_end->longitude}), bus, text_buses);
                    }
                }
                i = (i + 1) == static_cast<int>(color_palette_.size()) ? 0 : (i + 1);
                polyline.SetFillColor(svg::NoneColor);
                polyline.SetStrokeWidth(line_width_);
                polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                polyline.SetStrokeColor(color_palette_.at(i));
                bus = route.second;

                MakeTextForBus(color_palette_.at(i), point, bus, text_buses);
            }
            
            polyline.AddPoint(point);
            unique_stops.emplace(route.first);
        }
        if (polyline.SizePoints() != 0) {
            doc.Add(std::move(polyline));
            if (bus != nullptr && !bus->is_roundtrip && bus->stop_begin != bus->stop_end) {
                MakeTextForBus(color_palette_.at(i), Project({ bus->stop_end->latitude, bus->stop_end->longitude }), bus, text_buses);
            }
        }
        for (auto& text : text_buses) {
            doc.Add(std::move(text));
        }
        for (auto stop : unique_stops) {
            svg::Circle c;
            c.SetFillColor(svg::Color("white"s));
            c.SetRadius(stop_radius_);
            c.SetCenter(Project({ stop->latitude, stop->longitude }));
            doc.Add(std::move(c));
        }
        for (auto stop : unique_stops) {
            MakeTextForStop(Project({ stop->latitude, stop->longitude }), stop, doc);
        }
        doc.Render(out);
    }
}