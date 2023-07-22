#pragma once
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <deque>
#include <unordered_map>
#include <unordered_set>


#include "geo.h"


namespace transport_catalogue {
    using detail::geo::Coordinates;
    class TransportCatalogue {

        struct Stop {
            std::string name;
            Coordinates coordinates;
        };


        struct Bus {
            bool is_looped;
            std::string name;
            std::vector<const Stop*> route;
        };
        struct BusInfo {
            size_t stops_in_route;
            size_t unique_route;
            double route_length;
            double geography_length;
        };

        struct StringViewHasher {
            std::size_t operator()(std::string_view str) const {
                return std::hash<std::string_view>{}(str);
            }
        };
        struct HasherStop {
            std::size_t operator()(std::pair<const Stop*, const Stop* >stop) const {
                return std::hash<const void*>{}(stop.first) + std::hash<const void*>{}(stop.second);
            }
        };

    public:

        TransportCatalogue() = default;

        void AddStop(const std::string& name, Coordinates coordinates);

        void AddStopDistance(const Stop* stop_first, const Stop* stop_second, double distance);

        void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_loop);

        const Stop* FindStop(std::string_view stop_name) const;

        const Bus* FindBus(const std::string_view name) const;

        BusInfo GetBusInfo(std::string_view bus_name) const;

        std::set<std::string_view> GetStopInfo(std::string_view stop_name) const;

        double GetRealDistance(const Stop* from, const Stop* to) const;
    private:


        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Stop*, StringViewHasher> stopname_to_stop_;
        std::unordered_map<std::string_view, const Bus*, StringViewHasher> busname_to_busese_;
        std::unordered_map<std::string_view, std::set<std::string_view>, StringViewHasher> stopname_to_busese_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, double, HasherStop> stops_pair_to_distance_;





        bool IsValueName(std::string_view text);

        bool IsValidLatitude(double number);

        bool IsValidLongitude(double number);

        int GetNumberOfStops(const std::string_view name) const;

        int GetNumberOfUniqueStops(const std::string_view name) const;


    };
}
