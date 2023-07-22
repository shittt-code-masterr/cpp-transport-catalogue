#include "stat_reader.h"
namespace transport_catalogue {
    namespace stat_reader {

        std::string ParseBusInfo(const std::string& input) {

            size_t pos = input.find_first_not_of(' ');

            return input.substr(pos);
        }

        std::string ParseStopInfo(const std::string& input) {

            size_t pos = input.find_first_not_of(' ');

            return input.substr(pos);
        }

        void ParseRequestsOut(std::istream& input, std::ostream& output, TransportCatalogue& catalogue) {

            std::deque<input_reader::TransportRequest> requests;
            std::string line;
            int num_info_requests;
            input >> num_info_requests;
            input.ignore();

            for (int i = 0; i < num_info_requests; ++i) {
                std::getline(input, line);
                std::istringstream lineStream(line);
                std::string request_type;
                lineStream >> request_type;
                //std::getline(input, line);
                if (request_type == "Stop") {

                    requests.push_back({ input_reader::RequestType::GetStopInfo,line.substr(5) });
                }
                else if (request_type == "Bus") {

                    requests.push_back({ input_reader::RequestType::GetBusInfo, line.substr(4) });
                }
            }
            while (!requests.empty()) {
                auto request = requests.front();
                requests.pop_front();

                if (request.type == input_reader::RequestType::GetBusInfo) {
                    auto bus_name = ParseBusInfo(request.data);
                    try {

                        auto bus_info = catalogue.GetBusInfo(bus_name);
                        output << "Bus " << bus_name << ": " << bus_info.stops_in_route << " stops on route, "
                            << bus_info.unique_route << " unique stops, " << std::setprecision(6) << bus_info.route_lenght
                            << " route length, " << bus_info.geography_lenght << " curvature" << std::endl;
                    }
                    catch (const std::exception& e) {
                        output << "Bus " << bus_name << ": " << "not found" << std::endl;
                    }
                }
                else if (request.type == input_reader::RequestType::GetStopInfo) {
                    auto stop_name = ParseStopInfo(request.data);
                    try {
                        auto stop_info = catalogue.GetStopInfo(stop_name);
                        if (stop_info.empty()) {
                            output << "Stop " << stop_name << ": " << "no buses";
                        }
                        else {
                            output << "Stop " << stop_name << ": " << "buses ";
                            for (auto bus_name : stop_info) {
                                output << bus_name << " ";
                            }
                        }
                        output << std::endl;
                    }
                    catch (const std::exception& e) {
                        output << "Stop " << stop_name << ": " << "not found" << std::endl;
                    }
                }

            }
        }

        void ProcessRequests(std::istream& input, std::ostream& output) {

            auto catalogue = input_reader::ParseRequestsAdd(input);
            ParseRequestsOut(input, output, catalogue);

        }
    }
}