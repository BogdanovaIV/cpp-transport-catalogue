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

    Renderer::Renderer(domain::ParametersMap& parameters, double padding,
        std::pair<std::vector<geo::Coordinates>, std::vector<std::pair<const domain::Stop*, const domain::Bus*>>> CoordinatesAndStops_Buses):
        SphereProjector(CoordinatesAndStops_Buses.first.begin(), CoordinatesAndStops_Buses.first.end(), parameters.width, parameters.height, padding),
        parameters_(std::move(parameters)),
        Stops_Buses_(CoordinatesAndStops_Buses.second) {
    }

    void Renderer::MakeTextForBus(const svg::Color& color, const svg::Point& point, const domain::Bus* bus, std::vector<svg::Text>& text_buses) const {
        svg::Text text_background;
        text_background.SetFillColor(parameters_.underlayer_color);
        text_background.SetStrokeColor(parameters_.underlayer_color);
        text_background.SetStrokeWidth(parameters_.underlayer_width);
        text_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_background.SetPosition(point);
        text_background.SetOffset({ parameters_.bus_label_offset.first, parameters_.bus_label_offset.second });
        text_background.SetFontSize(parameters_.bus_label_font_size);
        text_background.SetFontFamily("Verdana"s);
        text_background.SetFontWeight("bold"s);
        text_background.SetData(bus->name);
        text_buses.push_back(std::move(text_background));

        svg::Text text_bus;
        text_bus.SetFillColor(color);
        text_bus.SetPosition(point);
        text_bus.SetOffset({ parameters_.bus_label_offset.first, parameters_.bus_label_offset.second });
        text_bus.SetFontSize(parameters_.bus_label_font_size);
        text_bus.SetFontFamily("Verdana"s);
        text_bus.SetFontWeight("bold"s);
        text_bus.SetData(bus->name);
        text_buses.push_back(std::move(text_bus));
    }

    void Renderer::MakeTextForStop(const svg::Point & point, const domain::Stop * stop, svg::Document& doc) const {
        svg::Text text_background;
        text_background.SetFillColor(parameters_.underlayer_color);
        text_background.SetStrokeColor(parameters_.underlayer_color);
        text_background.SetStrokeWidth(parameters_.underlayer_width);
        text_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_background.SetPosition(point);
        text_background.SetOffset({ parameters_.stop_label_offset.first, parameters_.stop_label_offset.second });
        text_background.SetFontSize(parameters_.stop_label_font_size);
        text_background.SetFontFamily("Verdana"s);
        text_background.SetData(stop->name);
        doc.Add(std::move(text_background));

        svg::Text text_bus;
        text_bus.SetFillColor(svg::Color{"black"s});
        text_bus.SetPosition(point);
        text_bus.SetOffset({ parameters_.stop_label_offset.first, parameters_.stop_label_offset.second });
        text_bus.SetFontSize(parameters_.stop_label_font_size);
        text_bus.SetFontFamily("Verdana"s);
        text_bus.SetData(stop->name);
        doc.Add(std::move(text_bus));
    }

    std::string Renderer::Render() const {
        svg::Document doc;
        int i = -1;
        std::vector<svg::Text> text_buses;
        const domain::Bus* bus = nullptr;
        svg::Polyline polyline;

        struct OrderingRule {
            bool operator()(const domain::Stop* lhs, const domain::Stop* rhs) const
            {
                return lhs->name < rhs->name;
            }
        };

        std::set<const domain::Stop*, OrderingRule> unique_stops;
        for (auto& route : Stops_Buses_) {
            svg::Point point = Project({ route.first->latitude, route.first->longitude });
            if (route.second != bus) {
                if (bus != nullptr) {
                    doc.Add(std::move(polyline));
                    polyline = svg::Polyline{};
                    if (!bus->is_roundtrip && bus->stop_begin != bus->stop_end) {
                        MakeTextForBus(parameters_.color_palette.at(i), Project({ bus->stop_end->latitude, bus->stop_end->longitude}), bus, text_buses);
                    }
                }
                i = (i + 1) == static_cast<int>(parameters_.color_palette.size()) ? 0 : (i + 1);
                polyline.SetFillColor(svg::NoneColor);
                polyline.SetStrokeWidth(parameters_.line_width);
                polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                polyline.SetStrokeColor(parameters_.color_palette.at(i));
                bus = route.second;

                MakeTextForBus(parameters_.color_palette.at(i), point, bus, text_buses);
            }
            
            polyline.AddPoint(point);
            unique_stops.emplace(route.first);
        }
        if (polyline.SizePoints() != 0) {
            doc.Add(std::move(polyline));
            if (bus != nullptr && !bus->is_roundtrip && bus->stop_begin != bus->stop_end) {
                MakeTextForBus(parameters_.color_palette.at(i), Project({ bus->stop_end->latitude, bus->stop_end->longitude }), bus, text_buses);
            }
        }
        for (auto& text : text_buses) {
            doc.Add(std::move(text));
        }
        for (auto stop : unique_stops) {
            svg::Circle c;
            c.SetFillColor(svg::Color("white"s));
            c.SetRadius(parameters_.stop_radius);
            c.SetCenter(Project({ stop->latitude, stop->longitude }));
            doc.Add(std::move(c));
        }
        for (auto stop : unique_stops) {
            MakeTextForStop(Project({ stop->latitude, stop->longitude }), stop, doc);
        }
        std::ostringstream out;
        doc.Render(out);
        return out.str();
    }
}