#include "serialization.h"

void Serialization::Serialize(transport_catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, transport_router::RouteSettings& route_setting) {
	std::ofstream out(serialization_settings_.file_name, std::ios::binary);
	if (!out)
		return;

	transport_proto::Container container;

	*(container.mutable_transport_catalogue()) = ConvertTransportCatalogue_(transport_catalogue);
	*(container.mutable_render_settings()) = ConvertRenderSettings_(map_renderer.GetSettings());
	*(container.mutable_route_settings()) = ConvertRouteSettings_(route_setting);
	container.SerializeToOstream(&out);
}

void Serialization::Deserialize(transport_catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, transport_router::RouteSettings& route_setting) {
	std::ifstream in(serialization_settings_.file_name, std::ios::binary);
	if (!in)
		return;

	transport_proto::Container container;
	if (!container.ParseFromIstream(&in))
		return;

	transport_catalogue = ConvertProtoTransportCatalogue_(*container.mutable_transport_catalogue());
	map_renderer.SetSetting(ConvertProtoRenderSettings_(*container.mutable_render_settings()));
	route_setting = (ConvertProtoRouteSettings_(*container.mutable_route_settings()));
}

void Serialization::SetSerializationSettings(SerializationSettings serialization_settings) {
	serialization_settings_ = serialization_settings;
}

transport_proto::TransportCatalogue Serialization::ConvertTransportCatalogue_(transport_catalogue::TransportCatalogue& transport_catalogue) {
	transport_proto::TransportCatalogue converted_catalogue;

	for (const auto& stop_ : transport_catalogue.GetStops()) {
		converted_catalogue.add_stops_list();
	}
	for (const auto& stop_ : transport_catalogue.GetStops()) {
		transport_proto::Stop* new_converted_stop = converted_catalogue.mutable_stops_list(stop_.id);

		new_converted_stop->set_name(stop_.name);
		new_converted_stop->set_coord_x(stop_.coordinates.lat);
		new_converted_stop->set_coord_y(stop_.coordinates.lng);
		new_converted_stop->set_id(stop_.id);
	}

	for (const auto& bus_ : transport_catalogue.GetBuses()) {
		transport_proto::Bus* new_converted_bus = converted_catalogue.add_buses_list();

		new_converted_bus->set_is_looped(bus_.is_looped);
		new_converted_bus->set_name(bus_.name);
		new_converted_bus->set_id(bus_.id);

		size_t count = (bus_.is_looped) ? bus_.route.size() : bus_.route.size() / 2 + 1;
		for (size_t i = 0; i < count; ++i) {
			new_converted_bus->add_stops(bus_.route.at(i).get()->id);
		}
	}

	for (auto& [pair, distance] : transport_catalogue.GetStopsPairToDistance()) {
		transport_proto::Pair* new_converted_pair = converted_catalogue.add_distances();

		new_converted_pair->set_id_from(pair.first.get()->id);
		new_converted_pair->set_id_to(pair.second.get()->id);
		new_converted_pair->set_distance(distance);
	}
	
	return converted_catalogue;
}

std::string Serialization::GetStopNameById_(transport_proto::TransportCatalogue& converted_catalogue, int id) {
	for (int i = 0; i < converted_catalogue.stops_list_size(); ++i) {
		const transport_proto::Stop& converted_stop = converted_catalogue.stops_list(i);
		if (converted_stop.id() == id)
			return converted_stop.name();
	}
	return "";
}

transport_catalogue::TransportCatalogue Serialization::ConvertProtoTransportCatalogue_(transport_proto::TransportCatalogue& converted_catalogue) {
	transport_catalogue::TransportCatalogue transport_catalogue;

	for (int i = 0; i < converted_catalogue.stops_list_size(); ++i) {
		const transport_proto::Stop& converted_stop = converted_catalogue.stops_list(i);
		transport_catalogue.AddStop(converted_stop.name(), geo::Coordinates{
			converted_stop.coord_x(),
			converted_stop.coord_y() });
	}

	for (int i = 0; i < converted_catalogue.buses_list_size(); ++i) {
		const transport_proto::Bus& converted_bus = converted_catalogue.buses_list(i);

		std::vector<std::string> stops;


		for (int j = 0; j < converted_bus.stops_size(); ++j) {
			stops.push_back(GetStopNameById_(converted_catalogue, converted_bus.stops(j)));
		}
		transport_catalogue.AddBus(converted_bus.name(), stops, converted_bus.is_looped());
	}

	for (int i = 0; i < converted_catalogue.distances_size(); ++i) {
		transport_catalogue.AddStopDistance(transport_catalogue.FindStop(
			GetStopNameById_(converted_catalogue, converted_catalogue.distances(i).id_from())),
			transport_catalogue.FindStop(GetStopNameById_(converted_catalogue, converted_catalogue.distances(i).id_to())),
			converted_catalogue.distances(i).distance()
		);

	}
	
	return transport_catalogue;
}

transport_proto::Color Serialization::ConvertColor_(svg::Color color) {
	transport_proto::Color converted_color;

	if (std::holds_alternative<svg::Rgb>(color)) {
		converted_color.set_index(1);
		converted_color.mutable_rgb()->set_red(std::get<svg::Rgb>(color).red);
		converted_color.mutable_rgb()->set_green(std::get<svg::Rgb>(color).green);
		converted_color.mutable_rgb()->set_blue(std::get<svg::Rgb>(color).blue);
	}
	else if (std::holds_alternative<svg::Rgba>(color)) {
		converted_color.set_index(2);
		converted_color.mutable_rgba()->set_red(std::get<svg::Rgba>(color).red);
		converted_color.mutable_rgba()->set_green(std::get<svg::Rgba>(color).green);
		converted_color.mutable_rgba()->set_blue(std::get<svg::Rgba>(color).blue);
		converted_color.mutable_rgba()->set_opacity(std::get<svg::Rgba>(color).opacity);
	}
	else if (std::holds_alternative<std::string>(color)) {
		converted_color.set_index(3);
		converted_color.set_text(std::get<std::string>(color));
	}
	else {
		converted_color.set_index(0);
	}

	return converted_color;
}

transport_proto::RenderSettings Serialization::ConvertRenderSettings_(renderer::RenderSettings render_settings) {
	transport_proto::RenderSettings converted_settings;

	converted_settings.set_width(render_settings.width);
	converted_settings.set_height(render_settings.height);
	converted_settings.set_padding(render_settings.padding);
	converted_settings.set_line_width(render_settings.line_width);
	converted_settings.set_stop_radius(render_settings.stop_radius);
	converted_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
	converted_settings.add_bus_label_offset(render_settings.bus_label_offset[0]);
	converted_settings.add_bus_label_offset(render_settings.bus_label_offset[1]);
	converted_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
	converted_settings.add_stop_label_offset(render_settings.stop_label_offset[0]);
	converted_settings.add_stop_label_offset(render_settings.stop_label_offset[1]);
	converted_settings.set_underlayer_width(render_settings.underlayer_width);

	*(converted_settings.mutable_underlayer_color()) = ConvertColor_(render_settings.underlayer_color);

	for (size_t i = 0; i < render_settings.color_palette.size(); ++i) {
		*(converted_settings.add_color_palette()) = ConvertColor_(render_settings.color_palette.at(i));
	}

	return converted_settings;
}

svg::Color Serialization::ConvertProtoColor_(transport_proto::Color converted_color) {
	svg::Color color;

	if (converted_color.index() == 1) {

		svg::Rgb rgb;
		rgb.red = static_cast<uint8_t>(converted_color.rgb().red());
		rgb.green = converted_color.rgb().green();
		rgb.blue = converted_color.rgb().blue();
		color = rgb;

	}
	else if (converted_color.index() == 2) {

		svg::Rgba rgba;
		rgba.red = converted_color.rgba().red();
		rgba.green = converted_color.rgba().green();
		rgba.blue = converted_color.rgba().blue();
		rgba.opacity = converted_color.rgba().opacity();
		color = rgba;

	}
	else if (converted_color.index() == 3) {

		color = converted_color.text();

	}

	return color;
}

renderer::RenderSettings Serialization::ConvertProtoRenderSettings_(transport_proto::RenderSettings converted_settings) {
	renderer::RenderSettings render_settings;

	render_settings.width = converted_settings.width();
	render_settings.height = converted_settings.height();
	render_settings.padding = converted_settings.padding();
	render_settings.line_width = converted_settings.line_width();
	render_settings.stop_radius = converted_settings.stop_radius();
	render_settings.bus_label_font_size = converted_settings.bus_label_font_size();
	render_settings.bus_label_offset[0] = converted_settings.bus_label_offset(0);
	render_settings.bus_label_offset[1] = converted_settings.bus_label_offset(1);
	render_settings.stop_label_font_size = converted_settings.stop_label_font_size();
	render_settings.stop_label_offset[0] = converted_settings.stop_label_offset(0);
	render_settings.stop_label_offset[1] = converted_settings.stop_label_offset(1);
	render_settings.underlayer_width = converted_settings.underlayer_width();

	render_settings.underlayer_color = ConvertProtoColor_(converted_settings.underlayer_color());

	render_settings.color_palette.clear();
	for (int i = 0; i < converted_settings.color_palette_size(); ++i) {
		render_settings.color_palette.push_back(ConvertProtoColor_(converted_settings.color_palette(i)));
	}

	return render_settings;
}

transport_proto::RouteSettings Serialization::ConvertRouteSettings_(transport_router::RouteSettings& route_settings) {
	transport_proto::RouteSettings converted_settings;

	converted_settings.set_bus_wait_time(route_settings.bus_wait_time);
	converted_settings.set_bus_velocity(route_settings.bus_velocity);

	return converted_settings;
}
transport_router::RouteSettings Serialization::ConvertProtoRouteSettings_(
	transport_proto::RouteSettings converted_settings) {
	transport_router::RouteSettings route_settings;
	route_settings.bus_wait_time = converted_settings.bus_wait_time();
	route_settings.bus_velocity = converted_settings.bus_velocity();
	return route_settings;
}
