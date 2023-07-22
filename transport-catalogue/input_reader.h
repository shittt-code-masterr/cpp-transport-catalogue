#pragma once
#include <iostream>
#include <sstream>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <iomanip>
#include "transport_catalogue.h"
namespace transport_catalogue {
    namespace input_reader {
        enum class RequestType {
            AddStop,
            AddBus,
            GetBusInfo,
            GetStopInfo,
        };


        struct TransportRequest {
            RequestType type;
            std::string data;
        };



        std::tuple<std::string, double, double, std::string > ParseStop(const std::string& input);


        std::tuple< std::string, std::vector<std::string>, bool> ParseBus(const std::string& text);


        TransportCatalogue ParseRequestsAdd(std::istream& input);

    }
}



