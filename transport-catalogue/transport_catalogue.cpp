#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const std::string& name, Coordinates coordinates) {
        using namespace std::string_literals;
        if (!IsValidLatitude(coordinates.lat)) {
            throw std::invalid_argument("latitude invvalue"s);
        }
        if (!IsValidLongitude(coordinates.lng)) {
            throw std::invalid_argument("longitude invvalue"s);
        }
        if (!IsValueName(name)) {
            throw std::invalid_argument("Stop name invvalue"s);
        }
        if (!FindStop(name)) {
            stops_.emplace_back(Stop{ std::move(name), coordinates });
            const Stop* stop_ptr = &stops_.back();

            stopname_to_stop_[stop_ptr->name] = stop_ptr;;
        }



    }

    void TransportCatalogue::AddStopDistance(const Stop* stop_first, const Stop* stop_second, double distance) {
        stops_pair_to_distance_[{stop_first, stop_second}] = distance;
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_loop) {
        using namespace std::string_literals;
        if (!IsValueName(name)) {
            throw std::invalid_argument("Bus name contains prohibited characters"s);
        }
        std::vector<const Stop*> stops;
        for (auto& stop : stop_names) {
            const auto& st = FindStop(stop);
            if (!st) {
                throw std::out_of_range("The stop is not in the catalog"s);
            }
            stops.push_back(st);

        }
        if (!is_loop) {
            for (int it = static_cast<int>(stop_names.size()) - 2; it != -1; it--) {
                const auto& st = FindStop(stop_names[it]);
                stops.push_back(st);
            }
        }

        Bus bus_ = { is_loop, std::move(name), std::move(stops) };
        Bus& bus1 = buses_.emplace_back(std::move(bus_));
        for (auto& stop : bus1.route) {
            stopname_to_busese_[stop->name].insert(std::move(bus1.name));

        }
        busname_to_busese_[bus1.name] = &bus1;
    };

    const TransportCatalogue::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
        return (stopname_to_stop_.find(stop_name) != stopname_to_stop_.end()) ? stopname_to_stop_.at(stop_name) : nullptr;
    }

    const TransportCatalogue::Bus* TransportCatalogue::FindBus(const std::string_view name) const {
        return (busname_to_busese_.find(name) != busname_to_busese_.end()) ? busname_to_busese_.at(name) : nullptr;
    }

    TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        auto bus = busname_to_busese_.at(bus_name);

        size_t size = GetNumberOfStops(bus_name);
        size_t u_size = GetNumberOfUniqueStops(bus_name);
        double real_length = 0;
        double geography_length = 0;

        for (size_t i = 1; i < static_cast<int>(bus->route.size()); ++i) {
            auto first = bus->route[i - 1];
            auto sec = bus->route[i];
            real_length += GetRealDistance(first, sec);
            geography_length += ComputeDistance(first->coordinates, sec->coordinates);
        }

        return { size , u_size , real_length, real_length / geography_length }; // Divide the length by 1000 to convert to kilometers
    }

    std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
        std::set<std::string_view> tmp_;
        if (stopname_to_stop_.find(stop_name) == stopname_to_stop_.end()) {
            throw(std::exception());
        }
        else if (stopname_to_busese_.find(stop_name) == stopname_to_busese_.end()) {
            return tmp_;
        }
        return stopname_to_busese_.at(stop_name);
    }


    double TransportCatalogue::GetRealDistance(const Stop* from, const Stop* to) const {
        if (stops_pair_to_distance_.find({ from, to }) == stops_pair_to_distance_.end()) {
            return stops_pair_to_distance_.at({ to, from });
        }
        return stops_pair_to_distance_.at({ from, to });
    }


    bool TransportCatalogue::IsValueName(std::string_view text) {
        return 	std::any_of(text.begin(), text.end(), [](char c) {
            return isalnum(c);
            });;
    }

    bool TransportCatalogue::IsValidLatitude(double number) {
        return -90.0 <= number && number <= 90.0;
    }

    bool TransportCatalogue::IsValidLongitude(double number) {
        return -180.0 <= number && number <= 180.0;
    }

    int TransportCatalogue::GetNumberOfStops(const std::string_view name) const {
        const auto bus = FindBus(name);
        return static_cast<int>(bus->route.size());
    }

    int TransportCatalogue::GetNumberOfUniqueStops(const std::string_view name) const {
        const auto bus = FindBus(name);
        std::unordered_set<std::string_view> result;
        for (const auto& stop : bus->route) {
            result.insert(stop->name);
        }
        return static_cast<int>(result.size());
    }

}
