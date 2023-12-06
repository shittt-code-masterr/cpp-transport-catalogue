#include "map_renderer.h"

namespace renderer {

   
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    MapRenderer::MapRenderer(RenderSettings setting) :
        setting_(setting)

    {

    }

    void MapRenderer::SetSetting(RenderSettings setting) {
        setting_ = setting;
    }

    void MapRenderer::BuildMap(transport_catalogue::TransportCatalogue& db_) {

        auto name_ = db_.GetBusName();
        
        std::set<std::string> stop_name;

        if (!name_.empty()) {
            std::vector <geo::Coordinates> all_coordinates;
            std::vector <std::vector<geo::Coordinates>> coordinates;
            std::map<std::string, geo::Coordinates> stopname_to_cooord;
            for (auto& name : name_) {
                auto bus = db_.FindBus(name);
                if (!bus.get()->route.empty()) {
                    std::vector <geo::Coordinates> route_coordinates;

                    for (auto stop_ : bus.get()->route) {

                        route_coordinates.push_back(stop_.get()->coordinates);
                        all_coordinates.push_back(stop_.get()->coordinates);
                        if (stopname_to_cooord.find(stop_.get()->name) == stopname_to_cooord.end()) {
                            stopname_to_cooord[stop_.get()->name] = stop_.get()->coordinates;
                        }
                    }
                    coordinates.push_back(route_coordinates);
                }
            }

            renderer::SphereProjector proj_ = AddRoute(all_coordinates, coordinates);
           
            BuildBusName(db_,proj_);
            BuildStops(stopname_to_cooord, proj_);
        }

    }

    RenderSettings MapRenderer::GetSettings() const
    {
        return setting_;
    }
   
    SphereProjector MapRenderer::AddRoute(std::vector<geo::Coordinates>& coord, std::vector< std::vector<geo::Coordinates>>& all_stop) {
        auto proj = AddAllRoute(coord);
        
        int start = 0;
        for (auto route : all_stop) {
            svg::Polyline line_;
            line_.SetFillColor(svg::NoneColor);
            line_.SetStrokeColor(setting_.color_palette[start % setting_.color_palette.size()]) ;
            line_.SetStrokeWidth(setting_.line_width);
            line_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            line_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            //SphereProjector proj(coord.begin(), coord.end(), setting_.width, setting_.height, setting_.padding);
            start++;
            for (auto coord_ : route) {
                auto point = proj(coord_);
                line_.AddPoint(point);
                }
            doc_.Add(line_);
        }

        return proj;
    }
    
    void MapRenderer::AddBusName(std::string& bus_name, geo::Coordinates begin, geo::Coordinates end, SphereProjector& proj,int start) {
        bool flag = (begin != end ? true : false);

        svg::Text text;
        svg::Text under_text;

        text.SetFillColor(setting_.color_palette[start % setting_.color_palette.size()]);

        text.SetPosition(proj(begin));
        under_text.SetPosition(proj(begin));
        text.SetOffset(svg::Point(setting_.bus_label_offset[0], setting_.bus_label_offset[1]));
        under_text.SetOffset(svg::Point(setting_.bus_label_offset[0], setting_.bus_label_offset[1]));
        text.SetFontSize(setting_.bus_label_font_size);
        under_text.SetFontSize(setting_.bus_label_font_size);
        text.SetFontFamily("Verdana");
        under_text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        under_text.SetFontWeight("bold");
        text.SetData(bus_name);
        under_text.SetData(bus_name);

        under_text.SetStrokeWidth(setting_.underlayer_width);
        under_text.SetFillColor(setting_.underlayer_color);
        under_text.SetStrokeColor(setting_.underlayer_color);
        under_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        under_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        doc_.Add(under_text);
        doc_.Add(text);
        
        if (flag) {
            svg::Text text_end  = text;
            svg::Text under_text_end = under_text;
            text_end.SetPosition(proj(end));
            under_text_end.SetPosition(proj(end));
            doc_.Add(under_text_end);
            doc_.Add(text_end);
            
        }

    }

    void MapRenderer::AddStop(std::map<std::string, geo::Coordinates>& stops, SphereProjector& proj) {
        for (const auto& [_, cord]: stops) {
            svg::Circle stop_point_;
            stop_point_.SetCenter(proj(cord));
            stop_point_.SetRadius(setting_.stop_radius);
            stop_point_.SetFillColor("white");
            doc_.Add(stop_point_);
        }   

        AddStopName(stops, proj);
    }

    void MapRenderer::AddStopName(std::map<std::string, geo::Coordinates>&stops, SphereProjector& proj) {
        for (const auto& [name, cord] : stops) {
            svg::Text text;
            svg::Text under_text;

            text.SetFillColor("black");
            text.SetPosition(proj(cord));
            under_text.SetPosition(proj(cord));
            text.SetOffset(svg::Point(setting_.stop_label_offset[0], setting_.stop_label_offset[1]));
            under_text.SetOffset(svg::Point(setting_.stop_label_offset[0], setting_.stop_label_offset[1]));
            text.SetFontSize(setting_.stop_label_font_size);
            under_text.SetFontSize(setting_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            under_text.SetFontFamily("Verdana");
            
            text.SetData(name);
            under_text.SetData(name);

            under_text.SetStrokeWidth(setting_.underlayer_width);
            under_text.SetFillColor(setting_.underlayer_color);
            under_text.SetStrokeColor(setting_.underlayer_color);
            under_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            under_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            doc_.Add(under_text);
            doc_.Add(text);
        
        }
    }

    std::ostream& MapRenderer::GetDocument(std::ostream& out) const {
         doc_.Render(out);
         return out;
    }

    SphereProjector MapRenderer::AddAllRoute(std::vector<geo::Coordinates>& all_route_) {
        return SphereProjector(all_route_.begin(), all_route_.end(), setting_.width, setting_.height, setting_.padding);
    }

    void MapRenderer::BuildBusName(transport_catalogue::TransportCatalogue& db_,renderer::SphereProjector proj_) {
        auto names = db_.GetBusName();
        if (!names.empty()) {
            int start = 0;
            for (auto name : names) {

                auto bus = db_.FindBus(name);
                if (!bus.get()->route.empty()) {
                    if (bus.get()->is_looped) {
                        AddBusName(bus.get()->name, bus.get()->route[0].get()->coordinates, bus.get()->route[0].get()->coordinates, proj_, start);
                    }
                    else {
                        int size_ = bus.get()->route.size();
                       AddBusName(bus.get()->name, bus.get()->route[0].get()->coordinates, bus.get()->route[size_ / 2].get()->coordinates, proj_, start);
                    }
                    start++;
                }
            }
        }

    }


    void MapRenderer::BuildStops(std::map<std::string, geo::Coordinates>& stops, renderer::SphereProjector proj_) {
        AddStop(stops, proj_);
    }
}
