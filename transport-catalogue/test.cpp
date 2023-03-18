#include"test.h"
#include <sstream>
#include <cassert>

using namespace std::string_literals;


void TestReadInput() {
	{
		std::string str = " 4 \n"s;
		str += "  Stop Tolstopaltsevo : 55.611087, 37.208290\n"s;
		str += "Stop   Marushkino: 55.595884, 37.209755   \n"s;
		str += "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"s;
		str += "Bus    750: Tolstopaltsevo - Marushkino - Rasskazovka   \n";

		std::istringstream input(str);
		std::vector<std::pair<KindOfRequest, std::string>> res = input_istream::Read(input, true);
		assert(res[0] == std::pair(KindOfRequest::AddStop, "Tolstopaltsevo : 55.611087, 37.208290"s));
		assert(res[1] == std::pair(KindOfRequest::AddStop, "Marushkino: 55.595884, 37.209755"s));
		assert(res[2] == std::pair(KindOfRequest::AddBus, "256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s));
		assert(res[3] == std::pair(KindOfRequest::AddBus, "750: Tolstopaltsevo - Marushkino - Rasskazovka"s));

	}
}

void TestFillTransportCatalogueIstream() {
	{
		std::string str = "13 \n"s;
		str += "  Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"s;
		str += "Stop    Marushkino   : 55.595884,  37.209755  , 9900m to Rasskazovka, 100m to Marushkino   \n"s;
		str += "Bus 256: Biryulyovo Zapadnoye >Biryusinka >Universam>    Biryulyovo Tovarnaya    > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"s;
		str += "Bus    750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka   \n";
		str += "Stop Rasskazovka: 55.632761, 37.333324, 9500m    to    Marushkino\n";
		str += "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n";
		str += "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n";
		str += "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n";
		str += "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n";
		str += "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n";
		str += "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n";
		str += "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n";
		str += "Stop Prazhskaya: 55.611678, 37.603831";

		std::istringstream input(str);
		std::vector<std::pair<KindOfRequest, std::string>> vector_request = input_istream::Read(input, true);
		auto res = input_istream::FillTransportCatalogue(vector_request);
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

		str = "3 \n"s;
		str += "Bus 256\n"s;
		str += "Bus 750\n"s;
		str += "Bus 751\n"s;

		std::istringstream output(str);
		std::vector<std::pair<KindOfRequest, std::string>> vector_request_out = input_istream::Read(output, false);
		auto request = vector_request_out[0].second;
		auto info = res.InfoBus(request);
		assert(info.Find == true);
		assert(info.Size == 6);
		assert(info.UniqueStop == 5);
		assert(info.Lenght == 5950);
		assert(std::round(info.crookedness * 100000) / 100000 == 1.36124);

		request = vector_request_out[1].second;
		info = res.InfoBus(request);
		assert(info.Find == true);
		assert(info.Size == 7);
		assert(info.UniqueStop == 3);
		assert(info.Lenght == 27400);
		assert(std::round(info.crookedness * 100000) / 100000 == 1.30853);

		request = vector_request_out[2].second;
		info = res.InfoBus(request);
		assert(info.Find == false);

		str = "2 \n"s;
		str += "Stop Prazhskaya\n"s;
		str += "Stop Biryulyovo Zapadnoye\n"s;

		std::istringstream outputStop(str);
		std::vector<std::pair<KindOfRequest, std::string>> vector_request_out_stop = input_istream::Read(outputStop, false);
		auto request_stop = vector_request_out_stop[0].second;
		auto infoStop = res.InfoStop(request_stop);
		assert(infoStop.empty() == true);

		request_stop = vector_request_out_stop[1].second;
		infoStop = res.InfoStop(request_stop);
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