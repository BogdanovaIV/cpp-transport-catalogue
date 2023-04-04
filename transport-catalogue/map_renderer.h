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
        // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // ���� ����� ����������� ����� �� ������, ��������� ������
            if (points_begin == points_end) {
                return;
            }

            // ������� ����� � ����������� � ������������ ��������
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // ������� ����� � ����������� � ������������ �������
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // ��������� ����������� ��������������� ����� ���������� x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // ��������� ����������� ��������������� ����� ���������� y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // ������������ ��������������� �� ������ � ������ ���������,
                // ���� ����������� �� ���
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *height_zoom;
            }
        }

        // ���������� ������ � ������� � ���������� ������ SVG-�����������
        svg::Point Project(geo::Coordinates coords) const;
        
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_; //������ ���� ����� �� ������ SVG-���������
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
        double width_ = 0.0;                          //������ ����������� � ��������
        double height_ = 0.0;                         //������ ����������� � ��������
        double line_width_ = 0.0;                     //������� �����
        double stop_radius_ = 0.0;                    //������ �����������
        int bus_label_font_size_ = 0;                 //������ ������, ������� �������� �������� ���������� ���������
        std::pair<double, double> bus_label_offset_;  //�������� ������� � ��������� �������� ������������ ��������� �������� ��������� �� �����
        int stop_label_font_size_ = 0;                //������ ������, ������� ������������ �������� ���������
        std::pair<double, double> stop_label_offset_; //�������� �������� ��������� ������������ � ��������� �� �����.
        svg::Color underlayer_color_;                 //���� �������� ��� ���������� ��������� � ���������
        double underlayer_width_ = 0.0;               //������� �������� ��� ���������� ��������� � ���������
        std::vector<svg::Color> color_palette_;       //�������� �������
        std::vector<std::pair<const transport::TransportCatalogue::Stop*, const transport::TransportCatalogue::Bus*>> Stops_Buses_; //����������� � ��������

        void MakeTextForBus(const svg::Color& color, const svg::Point& point, const transport::TransportCatalogue::Bus* bus, std::vector<svg::Text>& text_buses) const;

        void MakeTextForStop(const svg::Point& point, const transport::TransportCatalogue::Stop* stop, svg::Document& doc) const;

    };
}

