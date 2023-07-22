#pragma once
#include "input_reader.h"
namespace transport_catalogue {
	namespace stat_reader {
		std::string ParseBusInfo(const std::string& input);

		std::string ParseStopInfo(const std::string& input);

		void ParseRequestsOut(std::istream& input, std::ostream& output, TransportCatalogue& catalogue);

		void ProcessRequests(std::istream& input, std::ostream& output);
	}
}
