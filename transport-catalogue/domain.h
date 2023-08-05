#pragma once

#include<string>
#include<vector>
#include<memory>
#include"geo.h"

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};


struct Bus {
    bool is_looped;
    std::string name;
    std::vector<std::shared_ptr<Stop>> route;
};
struct BusInfo {
    int stops_in_route;
    int unique_route;
    double route_length;
    double geography_length;
};