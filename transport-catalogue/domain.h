#pragma once
#include <vector>
#include <variant>
#include "svg.h"

namespace domain {

    struct ParametersMap {
        double width = 0.0;                          //ширина изображения в пикселях
        double height = 0.0;                         //высота изображения в пикселях
        double line_width = 0.0;                     //толщина линий
        double stop_radius = 0.0;                    //радиус окружностей
        int bus_label_font_size = 0;                 //размер текста, которым написаны названия автобусных маршрутов
        std::pair<double, double> bus_label_offset{0.0, 0.0};  //смещение надписи с названием маршрута относительно координат конечной остановки на карте
        int stop_label_font_size = 0;                //размер текста, которым отображаются названия остановок
        std::pair<double, double> stop_label_offset{0.0, 0.0}; //смещение названия остановки относительно её координат на карте.
        svg::Color underlayer_color;                 //цвет подложки под названиями остановок и маршрутов
        double underlayer_width = 0.0;               //толщина подложки под названиями остановок и маршрутов
        std::vector<svg::Color> color_palette;       //цветовая палитра
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
    };

    struct BusInformation {
        bool Find;
        size_t Size = 0;
        int UniqueStop = 0;
        double Lenght = 0.0;
        double crookedness = 0.0;
    };

    struct RoutingSettings {
        int bus_velocity = 0;  //скорость автобуса, в км/ч
        int bus_wait_time = 0; //время ожидания автобуса на остановке, в минутах
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
