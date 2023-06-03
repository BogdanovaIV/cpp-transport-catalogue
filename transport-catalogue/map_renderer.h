#pragma once
#include "geo.h"
#include "domain.h"
#include <unordered_map>
#include "transport_catalogue.h"
#include <sstream>

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

    class Renderer {
    public:

        Renderer(domain::ParametersMap& parameters);

        std::string Render(const std::pair<std::vector<geo::Coordinates>, std::vector<std::pair<const domain::Stop*, const domain::Bus*>>>& CoordinatesAndStops_Buses) const;

    private:
        domain::ParametersMap parameters_;

        void MakeTextForBus(const svg::Color& color, const svg::Point& point, const domain::Bus* bus, std::vector<svg::Text>& text_buses) const;

        void MakeTextForStop(const svg::Point& point, const domain::Stop* stop, svg::Document& doc) const;

    };
}

