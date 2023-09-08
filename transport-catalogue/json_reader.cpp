#include "json_reader.h"





void JSONReader::ParseAddRequst(const json::Node& node) {
	using namespace std::literals;
	auto& map_ = node.AsDict();
	auto& type_ = map_.at("type").AsString();
	if (type_ == "Stop"s)
	{
		AddStopRequest tmp_ = { map_.at("name"s).AsString(), {map_.at("latitude"s).AsDouble(),map_.at("longitude"s).AsDouble()},map_.at("road_distances"s).AsDict() };
		requsts_[RequestType::AddStop].push_back(tmp_);
	}
	else if (type_ == "Bus"s)
	{
		AddBusRequest tmp_ = { map_.at("name"s).AsString(), map_.at("stops"s).AsArray(),map_.at("is_roundtrip"s).AsBool() };
		requsts_[RequestType::AddBus].push_back(tmp_);
	}

	
	
}

void JSONReader::ParseGetRequst( const json::Node& node) {
	using namespace std::literals;
	auto map_ = node.AsDict();
	auto type_ = map_.at("type").AsString();
	if (type_ == "Stop"s) {
		GetInfo tmp_ = { RequestType::GetStopInfo,map_.at("id"s).AsInt(),map_.at("name"s).AsString() };
		requsts_[RequestType::GetInfo].push_back(tmp_);

	}
	else if (type_ == "Bus"s)
	{
		GetInfo tmp_ = { RequestType::GetBusInfo, map_.at("id"s).AsInt(),map_.at("name"s).AsString() };
		requsts_[RequestType::GetInfo].push_back(tmp_);
	}
	else if (type_ == "Map"s) {
		GetInfo tmp_ = { RequestType::GetMap, map_.at("id"s).AsInt(),""};
		requsts_[RequestType::GetInfo].push_back(tmp_);
	}
	else if (type_ == "Route"s) {
		GetInfo tmp_ = { RequestType::GetRoute,map_.at("id"s).AsInt(),map_.at("from").AsString(),map_.at("to").AsString() };
		requsts_[RequestType::GetInfo].push_back(tmp_);
	}
}

void JSONReader::ParseJson(std::istream& input) {
	using namespace std::literals;
	
	
	auto node_ = json::Load(input).GetRoot().AsDict();
	if (node_.find("base_requests"s)!= node_.end()) {
		for (auto& n : node_.at("base_requests"s).AsArray()) {
			ParseAddRequst(n);
		}
	}
	if (node_.find("stat_requests"s) != node_.end()) {
		for (auto& n : node_.at("stat_requests"s).AsArray()) {
			ParseGetRequst(n);
		}
	}
	if (node_.find("render_settings"s) != node_.end()) {
		ParseRenderSetting(node_.at("render_settings"s).AsDict());
	}
	if (node_.find("routing_settings"s) != node_.end()) {
		auto tmp_ = node_.at("routing_settings"s).AsDict();
		requsts_[RequestType::AddSetting].push_back(AddSettingRequest{tmp_.at("bus_wait_time").AsInt(),tmp_.at("bus_velocity").AsDouble()});
	}
	
}

void JSONReader::ProcesAddRequest() {
	if (requsts_.find(RequestType::AddStop) != requsts_.end()) {
		for (auto request : requsts_.at(RequestType::AddStop)) {
			auto tmp_ = std::get<AddStopRequest>(request);
			db_.AddStop(tmp_.name, tmp_.coord);
		}
		for (auto request : requsts_.at(RequestType::AddStop)) {
			auto tmp_ = std::get<AddStopRequest>(request);
			for (auto stops_ : tmp_.stops_around) {
				db_.AddStopDistance(db_.FindStop(tmp_.name), db_.FindStop(stops_.first), stops_.second.AsDouble());
			}
		}
	}if (requsts_.find(RequestType::AddBus) != requsts_.end()) {
		for (auto request : requsts_.at(RequestType::AddBus)) {
			auto tmp_ = std::get<AddBusRequest>(request);
			std::vector<std::string> stops_names;
			for (auto stops_ : tmp_.route) {
				stops_names.push_back(stops_.AsString());
			}
			db_.AddBus(tmp_.name, stops_names, tmp_.is_loop);
		}
	}if (requsts_.find(RequestType::AddSetting) != requsts_.end()) {\

		auto tmp_ = std::get<AddSettingRequest>(*requsts_.at(RequestType::AddSetting).begin());
		db_.SetWaitTime(tmp_.wait_time);
		db_.SetVelocityTime(tmp_.velosity_time);
	
	}

	
}
json::Node JSONReader::BuildGetBusAnswer( GetInfo request) {
	using namespace std::literals;
	BusInfo info_ = db_.GetBusInfo(request.name);
	auto answer_ =
	json::Builder{}
		.StartDict()
			.Key("curvature"s).Value(info_.geography_length)
			.Key("request_id"s).Value(request.id)
			.Key("route_length"s).Value(info_.route_length)
			.Key("stop_count"s).Value(info_.stops_in_route)
			.Key("unique_stop_count"s).Value(info_.unique_route)
		.EndDict()
	.Build();

	return answer_;

}

json::Node JSONReader::BuildGetStopAnswer( GetInfo request) {
	using namespace std::literals;
	auto info_ = db_.GetStopInfo(request.name);
	json::Array buses;
	for (auto bus : info_) {
		buses.push_back(json::Node(std::string(bus)));
	}
	auto answer_ =
		json::Builder{}
			.StartDict()
				.Key("buses"s).Value(buses)
				.Key("request_id"s).Value(request.id)
			.EndDict()
		.Build();
	
	return answer_;

}

json::Node JSONReader::BuildGetRouteAnswer( GetInfo request, RequestHandler& manager) {
	using namespace std::literals;
	
	json::Array items_;
	auto route = manager.FindRoute(request.name, request.name_to);
	if (!route.has_value()) {
		throw(std::invalid_argument("not found"));
	}

	for (auto edge : route.value().edges) {
		if (edge.type == transport_router::EdgeType::Wait) {
			auto tmp_ = std::get<transport_router::EdgeWait>(edge.info);

			auto answer_=
			json::Builder{}
			.StartDict()

				.Key("type").Value("Wait")
				.Key("stop_name").Value(tmp_.name)
				.Key("time").Value(tmp_.weight)
				.EndDict().Build();
			items_.push_back(json::Node(answer_));
		
		}
		else {
			auto tmp_ = std::get<transport_router::EdgeBus>(edge.info);
			auto answer_ =
				json::Builder{}
				.StartDict()

				.Key("type").Value("Bus")
				.Key("bus").Value(tmp_.name)
				.Key("span_count").Value(static_cast<int>(tmp_.span_count))
				.Key("time").Value(tmp_.weight)
				.EndDict()
				.Build();
			items_.push_back(json::Node(answer_));


		}
		
	
	}

	auto result = json::Builder{}
		.StartDict()
		.Key("request_id").Value(request.id)
		.Key("total_time").Value(route.value().weight)
		.Key("items").Value(items_)
		.EndDict()
		.Build();

	return result;

}



json::Document JSONReader::ProcesGetRequest() {

	using namespace std::literals;

	RequestHandler manager(db_, map_);
	json::Array result;
	for (auto request : requsts_.at(RequestType::GetInfo)) {
		auto tmp_ = std::get<GetInfo>(request);


		switch (tmp_.type) {
		case RequestType::GetBusInfo: {
			try {
				json::Node answer_ = BuildGetBusAnswer(tmp_);
				result.push_back(json::Node(answer_));
			}
			catch (std::exception& e) {
				auto answer_ = 
				json::Builder{}
					.StartDict()
						.Key("request_id"s).Value(tmp_.id)
						.Key("error_message"s).Value("not found"s)
					.EndDict()
				.Build();
				result.push_back(json::Node(answer_));
			}
			break;
		}
		case RequestType::GetStopInfo: {
			try {
				json::Node answer_ = BuildGetStopAnswer( tmp_);
				result.push_back(json::Node(answer_));
			}
			catch (std::exception& e) {
				auto answer_ =
				json::Builder{}
					.StartDict()
						.Key("request_id"s).Value(tmp_.id)
						.Key("error_message"s).Value("not found"s)
					.EndDict()
				.Build();
				result.push_back(json::Node(answer_));
			}
			break;
		}
		case RequestType::GetMap: {


			std::ostringstream map_setting;
			manager.RenderMap(map_setting);


			auto answer_ =
				json::Builder{}
				.StartDict()
				.Key("map"s).Value(map_setting.str())
				.Key("request_id"s).Value(tmp_.id)
				.EndDict()
				.Build();


			result.push_back(answer_);
            break;
		}
							
		case RequestType::GetRoute: {
			try {
				json::Node answer_ = BuildGetRouteAnswer( tmp_, manager);
				result.push_back(json::Node(answer_));
			}
			catch (std::exception& e) {
				auto answer_ =
					json::Builder{}
					.StartDict()
					.Key("request_id"s).Value(tmp_.id)
					.Key("error_message"s).Value("not found"s)
					.EndDict()
					.Build();
				result.push_back(json::Node(answer_));
			}
			break;
		}
		default: {

			break;
		}
		}
	}
	return json::Document(result);
}




void JSONReader::ParseRenderSetting(const json::Dict& node) {
	renderer::RenderSettings settings;
	settings.width = node.at("width").AsDouble();
	settings.height = node.at("height").AsDouble();
	settings.padding = node.at("padding").AsDouble();
	settings.stop_radius = node.at("stop_radius").AsDouble();
	settings.line_width = node.at("line_width").AsDouble();
	settings.bus_label_font_size = node.at("bus_label_font_size").AsInt();
	settings.underlayer_width = (node.at("underlayer_width").AsDouble());
	// Parse bus_label_offset
	settings.bus_label_offset.clear();
	for (auto offset : node.at("bus_label_offset").AsArray()) {
		settings.bus_label_offset.push_back(offset.AsDouble());
	}

	settings.stop_label_font_size = node.at("stop_label_font_size").AsInt();

	// Parse stop_label_offset
	settings.stop_label_offset.clear();
	for (auto offset : node.at("stop_label_offset").AsArray()) {
		settings.stop_label_offset.push_back(offset.AsDouble());
	}

	settings.underlayer_color = ParseColor(node.at("underlayer_color"));

	// Parse color_palette
	settings.color_palette.clear(); 
	for (auto color : node.at("color_palette").AsArray()) {
		settings.color_palette.push_back(ParseColor(color));
	}
	map_.SetSetting(settings);

}

	
svg::Color JSONReader::ParseColor(const json::Node& color_) {
	svg::Color underlayer_color;

	if (color_.IsString()) {
		underlayer_color = color_.AsString();
	}
	else if (color_.IsArray()) {
		if (color_.AsArray().size() > 3) {
			svg::Rgba col;
			col.red = color_.AsArray().at(0).AsInt();
			col.green = color_.AsArray().at(1).AsInt();
			col.blue = color_.AsArray().at(2).AsInt();
			col.opacity = color_.AsArray().at(3).AsDouble();
			underlayer_color = col;
		}
		else {
			svg::Rgb col;
			col.red = color_.AsArray().at(0).AsInt();
			col.green = color_.AsArray().at(1).AsInt();
			col.blue = color_.AsArray().at(2).AsInt();
			underlayer_color = col;
		}
	}
	return underlayer_color;
}



void JSONReader::ProcessJsonRequests(std::istream& input, std::ostream& output) {

	ParseJson(input);
	ProcesAddRequest();
	auto answer = ProcesGetRequest();
	json::Print(answer, output);

}