#pragma once
#include <vector>
#include <variant>
#include "svg.h"

namespace domain {

    struct ParametersMap {
        double width = 0.0;                          //������ ����������� � ��������
        double height = 0.0;                         //������ ����������� � ��������
        double line_width = 0.0;                     //������� �����
        double stop_radius = 0.0;                    //������ �����������
        int bus_label_font_size = 0;                 //������ ������, ������� �������� �������� ���������� ���������
        std::pair<double, double> bus_label_offset{ 0.0, 0.0 };  //�������� ������� � ��������� �������� ������������ ��������� �������� ��������� �� �����
        int stop_label_font_size = 0;                //������ ������, ������� ������������ �������� ���������
        std::pair<double, double> stop_label_offset{ 0.0, 0.0 }; //�������� �������� ��������� ������������ � ��������� �� �����.
        svg::Color underlayer_color;                 //���� �������� ��� ���������� ��������� � ���������
        double underlayer_width = 0.0;               //������� �������� ��� ���������� ��������� � ���������
        std::vector<svg::Color> color_palette;       //�������� �������
        double padding = 0.0;
    };

    struct Stop {
        std::string name;
        double latitude;
        double longitude;
        size_t id;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> Route;
        const Stop* stop_begin = nullptr;
        const Stop* stop_end = nullptr;
        bool is_roundtrip = false;
        size_t id;
    };

    struct BusInformation {
        bool Find;
        size_t Size = 0;
        int UniqueStop = 0;
        double Lenght = 0.0;
        double crookedness = 0.0;
    };

    struct RoutingSettings {
        int bus_velocity = 0;  //�������� ��������, � ��/�
        int bus_wait_time = 0; //����� �������� �������� �� ���������, � �������
    };

    struct RouteItemWait {
        std::string_view stop_name;
        double time = 0.;
    };

    struct RouteItemBus {
        std::string_view bus;
        int span_count = 0;
        double time = 0.;
    };
    struct RouteInfo {
        double total_time = 0.;
        std::vector<std::variant<RouteItemWait, RouteItemBus>> items;
    };

    struct Route {
        const domain::Stop* from = nullptr;
        const domain::Stop* to = nullptr;
        double time = 0.;
        int span_count = 0;
    };

}
