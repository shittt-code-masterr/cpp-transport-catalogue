#pragma once

#include"transport_catalogue.h"
#include"map_renderer.h"




class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);

    
    std::shared_ptr<transport_catalogue::Bus> GetBusStat(const std::string_view& bus_name) const;

    
    std::set<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() ;

    

    
   
private:
    transport_catalogue::TransportCatalogue &db_;
    renderer::MapRenderer &renderer_;


   
};
