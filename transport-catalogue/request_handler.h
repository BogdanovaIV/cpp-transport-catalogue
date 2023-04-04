#pragma once
#include "input_reader.h"
#include "json_reader.h"
#include <iostream>
#include <iomanip>

namespace request_handler {
    enum KindOfStream {
        istream,
        json
    };

    void Read(std::istream& input, const KindOfStream& kind_stream);
    void Draw(std::istream& input, const KindOfStream& kind_stream);
}
