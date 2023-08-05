#pragma once
#include <iostream>
#include <sstream>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <iomanip>
#include <deque>
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "request_handler.h"
#include "domain.h"
enum class RequestType {
		AddStop,
		AddBus,
		GetInfo,
		GetBusInfo,
		GetStopInfo,
		GetMap
};
struct AddStopRequest
{
	std::string name;
	geo::Coordinates coord;
	json::Dict stops_around;

};
struct AddBusRequest
{
	std::string name;
	json::Array route;
	bool is_loop;
};

struct GetInfo{
	RequestType type;
	int id;
	
	std::string name;
};


class JSONReader {
public:
	using RequestMap = std::map<RequestType, std::vector<std::variant<AddStopRequest, AddBusRequest, GetInfo>>>;


	RequestMap& ParseAddRequst(RequestMap& requsts, const json::Node& node);

	RequestMap& ParseGetRequst(RequestMap& requsts, const json::Node& node);

	RequestMap ParseJson(std::istream& input, renderer::MapRenderer& map_for_setting);

	json::Node BuildGetBusAnswer(transport_catalogue::TransportCatalogue& catalogue, GetInfo request);

	json::Node BuildGetStopAnswer(transport_catalogue::TransportCatalogue& catalogue, GetInfo request);

	transport_catalogue::TransportCatalogue& ProcesAddRequest(RequestMap& requests, transport_catalogue::TransportCatalogue& catalogue);

	json::Document ProcesGetRequest(RequestMap& requests, transport_catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map);

	svg::Color ParseColor(const json::Node& color_);

	void ParseRenderSetting(const json::Dict& node, renderer::MapRenderer& map_for_setting);

	void ProcessJsonRequests(std::istream& input, std::ostream& output);
};
