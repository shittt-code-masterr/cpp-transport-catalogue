#include "json_reader.h"
#include "json_builder.h"
JSONReader::RequestMap& JSONReader::ParseAddRequst(RequestMap& requsts, const json::Node& node) {
	using namespace std::literals;
	auto& map_ = node.AsDict();
	auto& type_ = map_.at("type").AsString();
	if (type_ == "Stop"s)
	{
		AddStopRequest tmp_ = { map_.at("name"s).AsString(), {map_.at("latitude"s).AsDouble(),map_.at("longitude"s).AsDouble()},map_.at("road_distances"s).AsDict() };
		requsts[RequestType::AddStop].push_back(tmp_);
	}
	else if (type_ == "Bus"s)
	{
		AddBusRequest tmp_ = { map_.at("name"s).AsString(), map_.at("stops"s).AsArray(),map_.at("is_roundtrip"s).AsBool() };
		requsts[RequestType::AddBus].push_back(tmp_);
	}
	return requsts;
}

JSONReader::RequestMap& JSONReader::ParseGetRequst(RequestMap& requsts, const json::Node& node) {
	using namespace std::literals;
	auto map_ = node.AsDict();
	auto type_ = map_.at("type").AsString();
	if (type_ == "Stop"s) {
		GetInfo tmp_ = { RequestType::GetStopInfo,map_.at("id"s).AsInt(),map_.at("name"s).AsString() };
		requsts[RequestType::GetInfo].push_back(tmp_);

	}
	else if (type_ == "Bus"s)
	{
		GetInfo tmp_ = { RequestType::GetBusInfo, map_.at("id"s).AsInt(),map_.at("name"s).AsString() };
		requsts[RequestType::GetInfo].push_back(tmp_);
	}
	else if (type_ == "Map"s) {
		GetInfo tmp_ = { RequestType::GetMap, map_.at("id"s).AsInt(),""};
		requsts[RequestType::GetInfo].push_back(tmp_);
	}

	return requsts;
}

JSONReader::RequestMap JSONReader::ParseJson(std::istream& input,renderer::MapRenderer& map) {
	using namespace std::literals;
	RequestMap requsts;
	
	auto node_ = json::Load(input).GetRoot().AsDict();
	if (node_.find("base_requests"s)!= node_.end()) {
		for (auto& n : node_.at("base_requests"s).AsArray()) {
			ParseAddRequst(requsts, n);
		}
	}
	if (node_.find("stat_requests"s) != node_.end()) {
		for (auto& n : node_.at("stat_requests"s).AsArray()) {
			ParseGetRequst(requsts, n);
		}
	}
	if (node_.find("render_settings"s) != node_.end()) {
		ParseRenderSetting(node_.at("render_settings"s).AsDict(), map);
	}
	return requsts;
}

transport_catalogue::TransportCatalogue& JSONReader::ProcesAddRequest(RequestMap& requests, transport_catalogue::TransportCatalogue& catalogue) {
	if (requests.find(RequestType::AddStop) != requests.end()) {
		for (auto request : requests.at(RequestType::AddStop)) {
			auto tmp_ = std::get<AddStopRequest>(request);
			catalogue.AddStop(tmp_.name, tmp_.coord);
		}
		for (auto request : requests.at(RequestType::AddStop)) {
			auto tmp_ = std::get<AddStopRequest>(request);
			for (auto stops_ : tmp_.stops_around) {
				catalogue.AddStopDistance(catalogue.FindStop(tmp_.name), catalogue.FindStop(stops_.first), stops_.second.AsDouble());
			}
		}
	}if (requests.find(RequestType::AddBus) != requests.end()) {
		for (auto request : requests.at(RequestType::AddBus)) {
			auto tmp_ = std::get<AddBusRequest>(request);
			std::vector<std::string> stops_names;
			for (auto stops_ : tmp_.route) {
				stops_names.push_back(stops_.AsString());
			}
			catalogue.AddBus(tmp_.name, stops_names, tmp_.is_loop);
		}
	}
	return catalogue;
}
json::Node JSONReader::BuildGetBusAnswer(transport_catalogue::TransportCatalogue& catalogue, GetInfo request) {
	using namespace std::literals;
	BusInfo info_ = catalogue.GetBusInfo(request.name);
	auto answer =
	json::Builder{}
		.StartDict()
			.Key("curvature"s).Value(info_.geography_length)
			.Key("request_id"s).Value(request.id)
			.Key("route_length"s).Value(info_.route_length)
			.Key("stop_count"s).Value(info_.stops_in_route)
			.Key("unique_stop_count"s).Value(info_.unique_route)
		.EndDict()
	.Build();

	return answer;

}

json::Node JSONReader::BuildGetStopAnswer(transport_catalogue::TransportCatalogue& catalogue, GetInfo request) {
	using namespace std::literals;
	auto info_ = catalogue.GetStopInfo(request.name);
	json::Array buses;
	for (auto bus : info_) {
		buses.push_back(json::Node(std::string(bus)));
	}
	auto answer =
		json::Builder{}
			.StartDict()
				.Key("buses"s).Value(buses)
				.Key("request_id"s).Value(request.id)
			.EndDict()
		.Build();
	
	return answer;

}
json::Document JSONReader::ProcesGetRequest(RequestMap& requests, transport_catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& map) {

	using namespace std::literals;
	json::Array result;
	for (auto request : requests.at(RequestType::GetInfo)) {
		auto tmp_ = std::get<GetInfo>(request);


		switch (tmp_.type) {
		case RequestType::GetBusInfo: {
			try {
				json::Node answer_ = BuildGetBusAnswer(catalogue, tmp_);
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
				json::Node answer_ = BuildGetStopAnswer(catalogue, tmp_);
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
			
			RequestHandler manager(catalogue, map);
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
		
		}
		default: {

			break;
		}
		}
	}
	return json::Document(result);
}




void JSONReader::ParseRenderSetting(const json::Dict& node, renderer::MapRenderer& map_for_setting) {
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
	map_for_setting.SetSetting(settings);

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
	renderer::MapRenderer map_;
	auto i = ParseJson(input, map_);
	transport_catalogue::TransportCatalogue db;
	ProcesAddRequest(i, db);
	auto answer = ProcesGetRequest(i, db, map_);
	json::Print(answer, output);
	
	
}