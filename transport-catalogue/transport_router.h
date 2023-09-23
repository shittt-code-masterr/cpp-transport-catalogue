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
#include <memory>
#include <variant>

#include"domain.h"
#include"transport_catalogue.h"
#include"router.h"
#include"graph.h"



namespace transport_router {

	struct RouteSettings {
		int bus_wait_time; 
		double bus_velocity; 
	};

	const double HOURS_TO_MIN = 1000 / 60;
	enum class EdgeType
	{
		Wait,
		Bus,
	};

	struct EdgeBus
	{
		std::string name;
		size_t span_count;
		double weight = 0;
	};

	struct EdgeWait
	{
		std::string name;
		int weight = 0;
	};

	struct EdgeInfo
	{
		EdgeType type = EdgeType::Bus;
		std::variant<EdgeWait, EdgeBus> info;


	};

	struct RouteInfo {
		double weight;
		std::vector <EdgeInfo> edges;
	};

	using namespace transport_catalogue;
	class TransportRouter
	{
		using Graph = graph::DirectedWeightedGraph<double>;

	public:

		TransportRouter(TransportCatalogue& db, RouteSettings route_setting);

		std::optional<RouteInfo> FindRoute(std::shared_ptr<Stop>from, std::shared_ptr<Stop> to);

		RouteSettings GetRouteSettings() const;

		void SetSetting(RouteSettings set);


	private:
		TransportCatalogue& db_;
		RouteSettings setting_;;
		std::unordered_map<std::shared_ptr<Stop>, size_t>stop_to_id;
		std::unordered_map<graph::EdgeId, EdgeInfo>edge_id_to_edge;
		std::shared_ptr<graph::Router<double>> router_ = nullptr;
		Graph graph_;

		void BuildGraph();

		void AddBus(const Bus& bus);


		void AddBusEdges(const std::string& name, std::vector<std::shared_ptr<Stop> >::const_iterator begin, std::vector<std::shared_ptr<Stop> >::const_iterator const end);

	};


}

