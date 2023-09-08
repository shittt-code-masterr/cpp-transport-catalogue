#include"transport_router.h"



namespace transport_router {
	TransportRouter::TransportRouter(TransportCatalogue& db) :
		db_(db)

	{
		graph_ = std::move(Graph(db_.GetStops().size() * 2 + 1));
		BuildGraph();
		router_ = std::make_shared<graph::Router<double>>(graph::Router(graph_));
	}

	std::optional<transport_router::RouteInfo> TransportRouter::FindRoute(std::shared_ptr<Stop>from, std::shared_ptr<Stop> to) {
		graph::VertexId from_ = stop_to_id.at(from.get()->name);
		graph::VertexId to_ = stop_to_id.at(to.get()->name);


		auto route = router_.get()->BuildRoute(from_, to_);
		std::vector<transport_router::EdgeInfo> steps;
		if (route.has_value())
		{

			for (auto& step : route.value().edges)
			{
				steps.push_back(edge_id_to_edge[step]);
			}
		}
		transport_router::RouteInfo i = { route.value().weight, steps };
		return i;
	}





	void TransportRouter::BuildGraph() {

		for (auto stop : db_.GetStops()) {
			graph::Edge<double> edge_ = { (stop.id) * 2 , (stop.id) * 2 + 1, static_cast<double>(db_.GetWaitTime()) };



			stop_to_id[stop.name] = (stop.id * 2);
			
			edge_id_to_edge[graph_.AddEdge(edge_)] = { EdgeType::Wait,EdgeWait{stop.name,db_.GetWaitTime() } };
		}
		for (const Bus& bus : db_.GetBuses()) {
			AddBus(bus);
		}
	}

	void TransportRouter::AddBus(const Bus& bus) {
		if (bus.is_looped)
		{
			AddBusEdges(bus.name, bus.route.begin(), bus.route.end());
		}
		else {

			size_t end_route_num = (bus.route.size() - 1) / 2;
			auto end_route = bus.route.begin() + end_route_num;

			AddBusEdges(bus.name, bus.route.begin(), end_route + 1);
			AddBusEdges(bus.name, end_route, bus.route.end());

		}

	}


	void TransportRouter::AddBusEdges(const std::string& name, std::vector<std::shared_ptr<Stop> >::const_iterator begin, std::vector<std::shared_ptr<Stop> >::const_iterator const end) {
		for (auto stop1 = begin; stop1 < end; ++stop1) {
			double all_distance_time = 0;
			for (auto stop2 = stop1 + 1; stop2 < end; ++stop2) {
				
				
					double distance = db_.GetRealDistance(*(stop2 - 1), *stop2);
					double ride_time = distance / (db_.GetVelocityTime() * 1000 / 60);
					all_distance_time += ride_time;
					graph::Edge<double> edge_ = { (*stop1)->id * 2 + 1, (*stop2)->id * 2 , all_distance_time };
					edge_id_to_edge[graph_.AddEdge(edge_)] = { EdgeType::Bus,  EdgeBus{name,static_cast<size_t>(std::distance(stop1, stop2)),all_distance_time} };
				
					
					
				
			}
		}
	}
}