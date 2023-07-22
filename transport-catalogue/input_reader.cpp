#include "input_reader.h"
namespace transport_catalogue {
    namespace input_reader {


        std::tuple<std::string, double, double, std::string> ParseStop(const std::string& input) {
            size_t colon_pos = input.find(':');
            if (colon_pos == std::string::npos) {
                // Throw an exception or handle the case where ':' is not found.
                // For simplicity, we'll just return an empty tuple here.
                return {};
            }

            std::string name = input.substr(0, colon_pos);

            size_t comma_pos = input.find(',', colon_pos);
            double lat, lng;
            if (comma_pos != std::string::npos) {
                lat = std::stod(input.substr(colon_pos + 2, comma_pos - colon_pos - 2));
                lng = std::stod(input.substr(comma_pos + 2));
            }
            else {
                lat = std::stod(input.substr(colon_pos + 2));
                lng = 0.0; // Set longitude to 0 in case there's no ',' and only one coordinate provided.
            }

            // Truncate coordinates to 6 decimal places
            lat = std::round(lat * 1e6) / 1e6;
            lng = std::round(lng * 1e6) / 1e6;

            // Check if there's any additional information after the coordinates
            std::string line;
            if (comma_pos != std::string::npos) {
                size_t last_line = input.find(',', comma_pos + 1);
                if (last_line != std::string::npos) {
                    line = input.substr(last_line + 2);
                }
            }

            return { name, lat, lng, line };
        }
        std::unordered_map<std::string, double> ParseRawStop(std::string& input) {
            std::unordered_map<std::string, double> tmp_;
            size_t pos = 0;
            while (pos != std::string::npos) {
                size_t m_pos = input.find('m', pos);
                if (m_pos == std::string::npos) {
                    break; // No more 'm' characters found
                }
                double dist = std::stod(input.substr(pos, m_pos - pos));

                size_t comma_pos = input.find(',', m_pos);
                std::string name = input.substr(m_pos + 5, comma_pos - m_pos - 5);
                tmp_[name] = dist;
                if (comma_pos == std::string::npos) {
                    break; // No more 'm' characters found
                }
                else {
                    pos = comma_pos + 2;
                }


            }
            return tmp_;
        }
        std::tuple< std::string, std::vector<std::string>, bool> ParseBus(const std::string& text) {
            std::vector<std::string> words;
            words.reserve(text.size() / 2);
            std::string bus_name_;


            size_t wordStart = text.find(':');
            bus_name_ = std::string_view(text).substr(0, wordStart);
            wordStart += 2;
            for (std::size_t i = 0; i < text.size(); ++i) {

                if (text[i] == '>' || text[i] == '-') {

                    if (i > wordStart) {
                        words.emplace_back(text.substr(wordStart, i - wordStart - 1));
                    }
                    wordStart = i + 2;
                }
            }
            if (wordStart < text.size()) {
                words.emplace_back(text.substr(wordStart));
            }
            bool loop = text.find('>') == std::string::npos ? false : true;
            return { bus_name_ ,words, loop };
        }

        TransportCatalogue ParseRequestsAdd(std::istream& input) {
            TransportCatalogue catalogue;
            std::deque<TransportRequest> requests;
            std::string line;
            int num_requests;
            input >> num_requests;
            input.ignore();

            for (int i = 0; i < num_requests; ++i) {
                std::getline(input, line);
                std::istringstream lineStream(line);
                std::string request_type;
                lineStream >> request_type;

                if (request_type == "Stop") {
                    requests.push_front({ RequestType::AddStop,line.substr(5) });
                }
                else if (request_type == "Bus") {
                    requests.push_back({ RequestType::AddBus, line.substr(4) });
                }
            }
            std::unordered_map<std::string, std::unordered_map<std::string, double>> raw_stop_;
            while (!requests.empty()) {
                auto request = requests.front();
                requests.pop_front();

                if (request.type == RequestType::AddStop) {
                    auto [name, lat, lng, line_] = ParseStop(request.data);
                    try {
                        catalogue.AddStop(name, { lat,lng });
                        if (!line_.empty()) {
                            raw_stop_[name] = ParseRawStop(line_);
                        }
                    }
                    catch (const std::exception& e) {
                        throw(e.what());
                    }
                }
                else if (request.type == RequestType::AddBus) {
                    const auto [name, stops, loop] = ParseBus(request.data);
                    try {

                        catalogue.AddBus(name, stops, loop);
                    }
                    catch (const std::exception& e) {
                        throw(e.what());
                    }
                }
            }
            for (auto& [stop_name, map_] : raw_stop_) {
                auto stop_ = catalogue.FindStop(stop_name);
                for (auto& [name_, dist] : map_)
                {
                    catalogue.AddStopDistance(stop_, catalogue.FindStop(name_), dist);
                }
            }

            return catalogue;
        }

    }
}
