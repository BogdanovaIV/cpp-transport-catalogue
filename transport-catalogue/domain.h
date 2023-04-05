#pragma once
#include <vector>
#include "svg.h"

namespace domain {

    struct ParametersMap {
        double width = 0.0;                          //������ ����������� � ��������
        double height = 0.0;                         //������ ����������� � ��������
        double line_width = 0.0;                     //������� �����
        double stop_radius = 0.0;                    //������ �����������
        int bus_label_font_size = 0;                 //������ ������, ������� �������� �������� ���������� ���������
        std::pair<double, double> bus_label_offset{0.0, 0.0};  //�������� ������� � ��������� �������� ������������ ��������� �������� ��������� �� �����
        int stop_label_font_size = 0;                //������ ������, ������� ������������ �������� ���������
        std::pair<double, double> stop_label_offset{0.0, 0.0}; //�������� �������� ��������� ������������ � ��������� �� �����.
        svg::Color underlayer_color;                 //���� �������� ��� ���������� ��������� � ���������
        double underlayer_width = 0.0;               //������� �������� ��� ���������� ��������� � ���������
        std::vector<svg::Color> color_palette;       //�������� �������
        double padding = 0.0;
    };

    struct Stop {
        std::string name;
        double latitude;
        double longitude;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> Route;
        const Stop* stop_begin = nullptr;
        const Stop* stop_end = nullptr;
        bool is_roundtrip = false;
    };

    struct BusInformation {
        bool Find;
        size_t Size = 0;
        int UniqueStop = 0;
        double Lenght = 0.0;
        double crookedness = 0.0;
    };
}
