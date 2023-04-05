#include "test.h"
#include <sstream>
#include <cassert>

using namespace std::string_literals;

void TestFillTransportCatalogueJson() {
	{
		{
			std::ifstream input("Test.txt");
			if (input.is_open())
			{
			
				input_json::Json_reader json_reader{ input };
				transport::TransportCatalogue res = json_reader.MakeTransportCatalogue();

				auto* stop1 = res.FindStop("Ривьерский мост"s);
				assert(stop1->name == "Ривьерский мост"s);
				assert(stop1->latitude == 43.587795);
				assert(stop1->longitude == 39.716901);

				auto* stop2 = res.FindStop("Морской вокзал"s);
				assert(stop2->name == "Морской вокзал"s);
				assert(stop2->latitude == 43.581969);
				assert(stop2->longitude == 39.719848);

				auto* Bus1 = res.FindBus("114"s);
				assert(Bus1->name == "114"s);
				assert(Bus1->Route.at(0)->name == "Морской вокзал"s);
				assert(Bus1->Route.at(1)->name == "Ривьерский мост"s);
				assert(Bus1->Route.at(2)->name == "Морской вокзал"s);

				const json::Node& root = json_reader.GetRoot();
				const json::Node& stat_requests = root.AsMap().at("stat_requests"s);

				auto request = stat_requests.AsArray()[1];
				auto info = res.InfoBus(request.AsMap().at("name"s).AsString());
				assert(info.Find == true);
				assert(info.Size == 3);
				assert(info.UniqueStop == 2);
				assert(info.Lenght == 1700);
				assert(std::round(info.crookedness * 100000) / 100000 == 1.23199);

				auto request2 = stat_requests.AsArray()[0];
				auto infoStop = res.InfoStop(request2.AsMap().at("name"s).AsString());

				int i = 0;
				for (auto bus : infoStop) {
					if (i == 0) {
						assert(bus == "114");
					}
					++i;
				}

				auto distance = res.FindDistanceBetweenStops(res.FindStop("Морской вокзал"s), res.FindStop("Ривьерский мост"s));
				assert(distance == 850);

			}
			input.close();

		}
		{
			std::ifstream input("Test2.txt");
			if (input.is_open())
			{
				input_json::Json_reader json_reader{ input };
				transport::TransportCatalogue res = json_reader.MakeTransportCatalogue();

				auto* stop1 = res.FindStop("Tolstopaltsevo"s);
				assert(stop1->name == "Tolstopaltsevo"s);
				assert(stop1->latitude == 55.611087);
				assert(stop1->longitude == 37.208290);

				auto* stop2 = res.FindStop("Marushkino"s);
				assert(stop2->name == "Marushkino"s);
				assert(stop2->latitude == 55.595884);
				assert(stop2->longitude == 37.209755);

				auto* Bus1 = res.FindBus("256"s);
				assert(Bus1->name == "256"s);
				assert(Bus1->Route.at(0)->name == "Biryulyovo Zapadnoye"s);
				assert(Bus1->Route.at(1)->name == "Biryusinka"s);
				assert(Bus1->Route.at(2)->name == "Universam"s);
				assert(Bus1->Route.at(3)->name == "Biryulyovo Tovarnaya"s);
				assert(Bus1->Route.at(4)->name == "Biryulyovo Passazhirskaya"s);
				assert(Bus1->Route.at(5)->name == "Biryulyovo Zapadnoye"s);

				auto* Bus2 = res.FindBus("750"s);
				assert(Bus2->name == "750"s);
				assert(Bus2->Route.at(0)->name == "Tolstopaltsevo"s);
				assert(Bus2->Route.at(1)->name == "Marushkino"s);
				assert(Bus2->Route.at(2)->name == "Marushkino"s);
				assert(Bus2->Route.at(3)->name == "Rasskazovka"s);
				assert(Bus2->Route.at(4)->name == "Marushkino"s);
				assert(Bus2->Route.at(5)->name == "Marushkino"s);
				assert(Bus2->Route.at(6)->name == "Tolstopaltsevo"s);

				const json::Node& root = json_reader.GetRoot();
				const json::Node& stat_requests = root.AsMap().at("stat_requests"s);

				auto request = stat_requests.AsArray()[0];

				auto info = res.InfoBus(request.AsMap().at("name"s).AsString());
				assert(info.Find == true);
				assert(info.Size == 6);
				assert(info.UniqueStop == 5);
				assert(info.Lenght == 5950);
				assert(std::round(info.crookedness * 100000) / 100000 == 1.36124);

				request = stat_requests.AsArray()[1];
				info = res.InfoBus(request.AsMap().at("name"s).AsString());
				assert(info.Find == true);
				assert(info.Size == 7);
				assert(info.UniqueStop == 3);
				assert(info.Lenght == 27400);
				assert(std::round(info.crookedness * 100000) / 100000 == 1.30853);

				request = stat_requests.AsArray()[2];
				info = res.InfoBus(request.AsMap().at("name"s).AsString());
				assert(info.Find == false);

				auto request_stop = stat_requests.AsArray()[3];
				auto infoStop = res.InfoStop(request_stop.AsMap().at("name"s).AsString());
				assert(infoStop.empty() == true);

				request_stop = stat_requests.AsArray()[4];
				infoStop = res.InfoStop(request_stop.AsMap().at("name"s).AsString());
				int i = 0;
				for (auto bus : infoStop) {
					if (i == 0) {
						assert(bus == "256");
					}
					else {
						assert(bus == "828");
					}
					++i;
				}

				auto distance = res.FindDistanceBetweenStops(res.FindStop("Tolstopaltsevo"s), res.FindStop("Marushkino"s));
				assert(distance == 3900);

				distance = res.FindDistanceBetweenStops(res.FindStop("Rasskazovka"s), res.FindStop("Marushkino"s));
				assert(distance == 9500);
			}
		}
	}
}