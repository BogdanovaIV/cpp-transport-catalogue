#include "input_reader.h"
#include <utility>
#include <sstream>
#include <istream>

using namespace std::literals;


std::string input_istream::ReadLine(std::istream& input) {
    std::string s;
    std::getline(input, s);
    return s;
}

int input_istream::ReadLineWithNumber(std::istream& input) {
    int result;
    input >> result;
    ReadLine(input);
    return result;
}

std::vector<std::pair<transport::detail::KindOfRequest, std::string>> input_istream::Read(std::istream& input, bool FillCatalogue) {
    int count_request = ReadLineWithNumber(input);
    std::vector<std::pair<transport::detail::KindOfRequest, std::string>> vec_request(count_request);
    for (int i = 0; i < count_request; ++i) {
        std::string request = ReadLine(input);
        size_t begin_KindOfRequest = request.find_first_not_of(' ');
        size_t end_KindOfRequest = request.find_first_of(' ', begin_KindOfRequest);
        size_t size_KindOfRequest = end_KindOfRequest - begin_KindOfRequest;
        transport::detail::KindOfRequest kind_request;
        if (request.substr(begin_KindOfRequest, size_KindOfRequest) == "Bus"s) {
            kind_request = (FillCatalogue) ? transport::detail::KindOfRequest::AddBus : transport::detail::KindOfRequest::InfoBus;
        }
        else if (request.substr(begin_KindOfRequest, size_KindOfRequest) == "Stop"s) {
            kind_request = (FillCatalogue) ? transport::detail::KindOfRequest::AddStop : transport::detail::KindOfRequest::InfoStop;
        }
        else {
            throw std::invalid_argument("Request "s + request.substr(begin_KindOfRequest, end_KindOfRequest) + " don't find."s);
        }
        size_t begin_Request = request.find_first_not_of(' ', end_KindOfRequest + 1);
        size_t end_Request = request.find_last_not_of(' ');
        size_t size_Request = end_Request - begin_Request + 1;
        vec_request[i] = { kind_request, std::move(request.substr(begin_Request, size_Request)) };
    }
    return vec_request;
}

transport::TransportCatalogue input_istream::MakeTransportCatalogue(const std::vector<std::pair<transport::detail::KindOfRequest, std::string>>& vec_request) {
    transport::TransportCatalogue res;

    std::map<std::pair<std::string, std::string>, int> vector_info_distance_stop;

    for (auto& [kind_request, request] : vec_request) {
        if (kind_request == transport::detail::KindOfRequest::AddStop) {

            size_t begin_place = request.find_first_of(':');
            size_t end_name = request.find_last_not_of(' ', begin_place - 1);
            std::string name = std::move(request.substr(0, end_name + 1));

            std::stringstream stream(std::move(request.substr(begin_place + 1)));
            char delimiter = ',';
            std::vector<std::string> stops;
            std::string stop;

            std::getline(stream, stop, delimiter);
            double latitude = std::stod(stop);

            std::getline(stream, stop, delimiter);
            double longitude = std::stod(stop);

            while (std::getline(stream, stop, delimiter)) {
                size_t symbol_m = stop.find_first_of('m');
                int distance = std::stod(stop.substr(0, symbol_m));

                std::string place = std::move(stop.substr(symbol_m + 1));
                size_t symbol_t = place.find_first_of('t');

                place = std::move(place.substr(symbol_t + 2));

                size_t begin_stop = place.find_first_not_of(' ');
                size_t end_stop = place.find_last_not_of(' ');
                size_t size_stop = end_stop - begin_stop + 1;

                vector_info_distance_stop[std::pair{ name, place.substr(begin_stop, size_stop) }] = distance;
                if (!vector_info_distance_stop.count(std::pair{ std::move(place.substr(begin_stop, size_stop)), name })) {
                    vector_info_distance_stop[std::pair{ std::move(place.substr(begin_stop, size_stop)), name }] = distance;
                }
            }

            res.AddStop(name, latitude, longitude);
        }
    }

    for (auto info : vector_info_distance_stop) {
        res.AddDistanceBetweenStops(info.first.first, info.first.second, info.second);
    }

    for (auto& [kind_request, request] : vec_request) {
        if (kind_request == transport::detail::KindOfRequest::AddBus) {

            size_t begin_stop = request.find_first_of(':');
            size_t end_bus = request.find_last_not_of(' ', begin_stop - 1);
            std::string bus_number = std::move(request.substr(0, end_bus + 1));

            size_t end_stop = request.size() - (begin_stop + 1);

            auto it = request.find('>');
            bool round = (it != std::string::npos) ? true : false;
            std::stringstream stream(std::move(request.substr(begin_stop + 1, end_stop)));
            char delimiter = (round) ? '>' : '-';
            std::vector<std::string> stops;
            std::string stop;
            while (std::getline(stream, stop, delimiter)) {
                size_t begin_stop = stop.find_first_not_of(' ');
                size_t end_stop = stop.find_last_not_of(' ');
                size_t size_stop = end_stop - begin_stop + 1;
                stops.push_back(std::move(stop.substr(begin_stop, size_stop)));
            }
            res.AddBus(bus_number, stops, round);
        }
    }

    return res;
}

void input_istream::Reguest(transport::TransportCatalogue& TCatalogue, std::vector<std::pair<transport::detail::KindOfRequest, std::string>>& vec_request_out)
{
    for (auto& [kind_request, request] : vec_request_out) {
        if (kind_request == transport::detail::KindOfRequest::InfoBus) {
            auto info = TCatalogue.InfoBus(request);
            if (info.Find) {
                std::cout << "Bus "sv << request << ": "sv;
                std::cout << info.Size << " stops on route, "sv;
                std::cout << info.UniqueStop << " unique stops, "sv;
                std::cout << std::setprecision(6) << info.Lenght << " route length, "sv;
                std::cout << std::setprecision(6) << info.crookedness << " curvature"sv << std::endl;

            }
            else {
                std::cout << "Bus "sv << request << ": "sv;
                std::cout << "not found"sv << std::endl;
            }
        }
        else if (kind_request == transport::detail::KindOfRequest::InfoStop) {
            try {
                if (TCatalogue.FindStop(request)) {
                    auto info = TCatalogue.InfoStop(request);
                    if (info.empty()) {
                        std::cout << "Stop "sv << request << ": "sv;
                        std::cout << "no buses"sv << std::endl;

                    }
                    else {
                        std::cout << "Stop "sv << request << ": buses"sv;
                        for (auto bus : info) {
                            std::cout << " "sv << bus;
                        }
                        std::cout << std::endl;
                    }
                }
            }
            catch (std::out_of_range const&) {
                std::cout << "Stop "sv << request << ": "sv;
                std::cout << "not found"sv << std::endl;
            }
        }
    }
}

void input_istream::ReadStream(std::istream& input) {
    std::vector<std::pair<transport::detail::KindOfRequest, std::string>> vec_request = Read(input, true);
    transport::TransportCatalogue TCatalogue = MakeTransportCatalogue(vec_request);
    std::vector<std::pair<transport::detail::KindOfRequest, std::string>> vec_request_out = Read(input, false);
    Reguest(TCatalogue, vec_request_out);
}
