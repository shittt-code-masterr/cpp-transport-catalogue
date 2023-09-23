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
#include <memory>
#include <filesystem>
#include <fstream>  
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "request_handler.h"
#include "domain.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

enum class RequestType {
		AddStop,
		AddBus,
		AddSetting,
		GetInfo,
		GetBusInfo,
		GetStopInfo,
		GetMap,
		GetRoute,

		
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

struct AddSettingRequest
{
	
	int wait_time ;
	double velosity_time;

};

struct GetInfo{
	RequestType type;
	int id;
	std::string name;
	std::string name_to="";
	
};



class JSONReader {
	using Path = std::filesystem::path;
public:
	
	
	using RequestMap = std::map<RequestType, std::vector<std::variant<AddStopRequest, AddBusRequest, GetInfo, AddSettingRequest>>>;

	void ProcessJsonRequests(std::istream& input, std::ostream& output, bool is_make_base);

private:
	transport_catalogue::TransportCatalogue db_;
	renderer::MapRenderer map_;
	RequestMap requsts_;
	Serialization serial_;

	transport_router::RouteSettings route_setting;
	

	std::filesystem::path serialization_setting_;
	
	bool is_make_base;
	void ParseAddRequst(const json::Node& node);

	void ParseGetRequst(const json::Node& node);

	void ParseJson(std::istream& input);

	json::Node BuildGetBusAnswer(GetInfo request);

	json::Node BuildGetStopAnswer(GetInfo request);

	void ProcesAddRequest();

	json::Document ProcesGetRequest();

	svg::Color ParseColor(const json::Node& color_);

	void ParseRenderSetting(const json::Dict& node);

	json::Node BuildGetRouteAnswer(GetInfo request, RequestHandler& manager);

	
};
