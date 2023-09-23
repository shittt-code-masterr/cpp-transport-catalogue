#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <iomanip>
#include"geo.h"

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    size_t id = 0;
};


struct Bus {
    bool is_looped;
    std::string name;
    std::vector<std::shared_ptr<Stop>> route;
    size_t id = 0;
};
struct BusInfo {
    int stops_in_route;
    int unique_route;
    double route_length;
    double geography_length;
};
