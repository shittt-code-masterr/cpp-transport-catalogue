#include "request_handler.h"


RequestHandler::RequestHandler( transport_catalogue::TransportCatalogue& db,  renderer::MapRenderer& renderer):
	db_(db),
	renderer_(renderer)
{
}

// Возвращает информацию о маршруте (запрос Bus)
std::shared_ptr<transport_catalogue::Bus> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return db_.FindBus(bus_name);
}

// Возвращает маршруты, проходящие через
 std::set<std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	return db_.GetStopInfo(stop_name);
}



svg::Document RequestHandler::RenderMap()  {
	renderer_.BuildMap(db_);
	return renderer_.GetDocument();
}

