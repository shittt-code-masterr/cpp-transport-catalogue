#pragma once

#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <iterator>
#include <set>
#include <map>
#include "svg.h"
#include "geo.h"
#include "transport_catalogue.h"



namespace renderer {
    
    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) ;


    struct RenderSettings {
        
        

        double width = 1200.0;
        double height = 1200.0;
        double padding = 50.0;
        double line_width = 14.0;
        double stop_radius = 5.0;
        int bus_label_font_size = 20;
        std::vector<double> bus_label_offset = { 7.0, 15.0 };
        int stop_label_font_size = 20;
        std::vector<double> stop_label_offset = { 7.0, -3.0 };
        svg::Color underlayer_color = svg::Rgba(255, 255, 255, 0.85);
        double underlayer_width = 3.0;
        std::vector<svg::Color> color_palette{ "green", svg::Rgb(255, 160, 0), "red" };
        
    };

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates

        

        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:
        MapRenderer() = default;
        MapRenderer(const MapRenderer&) = delete;

        MapRenderer(RenderSettings setting);

        void SetSetting(RenderSettings setting);

        std::ostream&GetDocument(std::ostream& out) const ;

        void BuildMap(transport_catalogue::TransportCatalogue& db_);

    private:

        RenderSettings setting_;
        svg::Document doc_;
        
        SphereProjector AddAllRoute(std::vector<geo::Coordinates>& all_route_);

        SphereProjector AddRoute(std::vector<geo::Coordinates>& coord, std::vector< std::vector<geo::Coordinates>>& all_stop);

        void AddBusName(std::string& bus_name, geo::Coordinates begin, geo::Coordinates end, SphereProjector& proj, int start);

        void AddStop(std::map<std::string, geo::Coordinates>& stops, SphereProjector& proj);

        void AddStopName(std::map<std::string, geo::Coordinates>& stops, SphereProjector& proj);
        
        void BuildBusName(transport_catalogue::TransportCatalogue& db_, renderer::SphereProjector proj_);

        void BuildStops(std::map<std::string, geo::Coordinates>& stops, renderer::SphereProjector proj_);
        
    };
}//renderer

