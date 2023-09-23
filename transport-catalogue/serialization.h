#pragma once

#include <filesystem>
#include <fstream>  
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <variant>

#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "svg.pb.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "transport_router.pb.h"
#include "svg.h"
#include "domain.h"

using Path = std::filesystem::path;

struct SerializationSettings {
	Path file_name;
};

class Serialization {
public:
	Serialization() = default;

	void Serialize(transport_catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, transport_router::RouteSettings& route_setting);

	void Deserialize(transport_catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, transport_router::RouteSettings& route_setting);

	void SetSerializationSettings(SerializationSettings serialization_settings);

private:
	SerializationSettings serialization_settings_;

	transport_proto::TransportCatalogue ConvertTransportCatalogue_(transport_catalogue::TransportCatalogue& transport_catalogue);

	std::string GetStopNameById_(transport_proto::TransportCatalogue& converted_catalogue, int id);
	transport_catalogue::TransportCatalogue ConvertProtoTransportCatalogue_(transport_proto::TransportCatalogue& converted_catalogue);


	transport_proto::Color ConvertColor_(svg::Color color);
	transport_proto::RenderSettings ConvertRenderSettings_(renderer::RenderSettings render_settings);

	svg::Color ConvertProtoColor_(transport_proto::Color converted_color);
	renderer::RenderSettings ConvertProtoRenderSettings_(transport_proto::RenderSettings converted_settings);

	transport_proto::RouteSettings ConvertRouteSettings_(transport_router::RouteSettings& route_settings);

	transport_router::RouteSettings ConvertProtoRouteSettings_(transport_proto::RouteSettings converted_settings);
};