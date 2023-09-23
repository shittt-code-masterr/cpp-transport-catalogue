#pragma once

#include"transport_catalogue.h"
#include"map_renderer.h"
#include "domain.h"
#include "transport_router.h"


class RequestHandler {
public:
    
    RequestHandler(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, transport_router::RouteSettings route_setting);

    
    std::shared_ptr<Bus> GetBusStat(const std::string_view bus_name) const;

    
    std::set<std::string_view> GetBusesByStop(const std::string_view stop_name) const;

    std::ostream&  RenderMap(std::ostream& out) const;

    
    
    std::optional<transport_router::RouteInfo>FindRoute(const std::string_view from, const std::string_view to)  ;
    
    
   
private:
    transport_catalogue::TransportCatalogue &db_;
    renderer::MapRenderer &renderer_;
    transport_router::TransportRouter router_;


   
};
