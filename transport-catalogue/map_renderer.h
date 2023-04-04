#pragma once
#include "geo.h"
#include "svg.h"
#include <unordered_map>
#include "transport_catalogue.h"

namespace map_renderer {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
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
        svg::Point Project(geo::Coordinates coords) const;
        
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_; //отступ краёв карты от границ SVG-документа
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class map: public SphereProjector {
    public:
        
        map(std::unordered_map<std::string, double>& parametrs, std::pair<double, double>& bus_label_offset,
            std::pair<double, double>& stop_label_offset, svg::Color& underlayer_color, std::vector<svg::Color>& color_palette,
            std::vector<geo::Coordinates>& coordinates, std::vector<std::pair<const transport::TransportCatalogue::Stop*, const transport::TransportCatalogue::Bus*>>& Stops_Buses);

        void Render(std::ostream& out) const;

    private:
        double width_ = 0.0;                          //ширина изображения в пикселях
        double height_ = 0.0;                         //высота изображения в пикселях
        double line_width_ = 0.0;                     //толщина линий
        double stop_radius_ = 0.0;                    //радиус окружностей
        int bus_label_font_size_ = 0;                 //размер текста, которым написаны названия автобусных маршрутов
        std::pair<double, double> bus_label_offset_;  //смещение надписи с названием маршрута относительно координат конечной остановки на карте
        int stop_label_font_size_ = 0;                //размер текста, которым отображаются названия остановок
        std::pair<double, double> stop_label_offset_; //смещение названия остановки относительно её координат на карте.
        svg::Color underlayer_color_;                 //цвет подложки под названиями остановок и маршрутов
        double underlayer_width_ = 0.0;               //толщина подложки под названиями остановок и маршрутов
        std::vector<svg::Color> color_palette_;       //цветовая палитра
        std::vector<std::pair<const transport::TransportCatalogue::Stop*, const transport::TransportCatalogue::Bus*>> Stops_Buses_; //остатоновки и маршруты

        void MakeTextForBus(const svg::Color& color, const svg::Point& point, const transport::TransportCatalogue::Bus* bus, std::vector<svg::Text>& text_buses) const;

        void MakeTextForStop(const svg::Point& point, const transport::TransportCatalogue::Stop* stop, svg::Document& doc) const;

    };
}

