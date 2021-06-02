/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <map>
#include <cv/cv.h>
#include "weeder/Pipeline.h"
#include "astar/AStar.hpp"

namespace romi {

        static const size_t kAstarResolution = 9;
        
        Pipeline::Pipeline(std::unique_ptr<IImageCropper>& cropper,
                           std::unique_ptr<IImageSegmentation>& segmentation,
                           std::unique_ptr<IConnectedComponents>& connected_components,
                           std::unique_ptr<IPathPlanner>& planner)
                : cropper_(),
                  segmentation_(),
                  connected_components_(),
                  planner_()
        {
                cropper_ = std::move(cropper);
                segmentation_ = std::move(segmentation);
                connected_components_ = std::move(connected_components);
                planner_ = std::move(planner);
        }

        std::vector<Path> Pipeline::run(ISession& session, Image& camera,
                                        double tool_diameter)
        {
                Image crop;
                crop_image(session, camera, tool_diameter, crop);
                session.store_png("crop", crop);

                Image mask;
                create_mask(session, crop, mask);
                session.store_png("segmentation", mask);

                romi::filter_mask(mask, mask, 8);
                session.store_png("mask", mask);

                Image dilated_mask;
                
                // TODO
                if (false) {
                        mask.dilate(kAstarResolution, dilated_mask);
                        session.store_png("dilated-mask", dilated_mask);
                } else {
                        dilated_mask = mask;
                }
                
                Image components;
                connected_components_->compute(session, dilated_mask, components);
                session.store_png("components", components);

                double diameter_pixels = cropper_->map_meters_to_pixels(tool_diameter
                                                                        + 0.010);
                size_t max_centers = (size_t) ((double) (mask.width() * mask.height())
                                               / (diameter_pixels * diameter_pixels));

                Centers centers = romi::calculate_centers(mask, max_centers);

                {
                        rpp::MemBuffer buffer;
                        for (auto & center: centers)
                                buffer.printf("%zu\t%zu\n", center.first, center.second);
                        session.store_txt("centers", buffer.tostring());
                }

                std::vector<Centers> component_centers
                        = romi::sort_centers(centers, components);

                char filename[64];
                std::vector<Path> paths;
                
                for (size_t i = 0; i < component_centers.size(); i++) {

                        // Compute shortest path through centers
                        Path initial_path = trace_path(session, component_centers[i], mask);
                        snprintf(filename, sizeof(filename), "path-initial-%02zu", i);
                        session.store_path(filename, 0, initial_path);

                        // check path for plant crossings
                        Path path;
                        check_path(session, mask, initial_path, path, i);
                        snprintf(filename, sizeof(filename), "path-%02zu", i);
                        session.store_path(filename, 0, path);

                        paths.emplace_back(path);
                }

                // double meters_to_pixels = cropper_->map_meters_to_pixels(1.0);

                // Path initial_path;
                // trace_path(session, mask, tool_diameter, meters_to_pixels, initial_path);
                // session.store_path("path0", 0, initial_path);

                // Path path;
                // check_path(session, mask, initial_path, path);
                // session.store_path("path", 0, path);

                // std::vector<Path> paths;
                // paths.emplace_back(path);

                return paths;
        }

        void Pipeline::crop_image(ISession& session, Image& camera,
                                  double tool_diameter, Image& crop)
        {
                if (!cropper_->crop(session, camera, tool_diameter, crop)) {
                        throw std::runtime_error("Pipeline: crop failed");
                }
        }

        void Pipeline::create_mask(ISession& session, Image &crop, Image &mask)
        {
                if (!segmentation_->create_mask(session, crop, mask)) {
                        throw std::runtime_error("Pipeline: segmentation failed");
                }
        }

        Path Pipeline::trace_path(ISession& session, Centers& centers, Image &mask)
        {
                return planner_->trace_path(session, centers, mask);
        }

        void Pipeline::check_path(ISession& session, Image& mask, Path& path,
                                  Path& result, size_t index)
        {
                rpp::MemBuffer buffer;
                int w = (int) mask.width();
                int h = (int) mask.height();

                buffer.printf("<?xml version=\"1.0\" "
                              "encoding=\"UTF-8\" standalone=\"no\"?>"
                              "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
                              "xmlns=\"http://www.w3.org/2000/svg\" "
                              "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                              "version=\"1.0\" "
                              "width=\"%dpx\" height=\"%dpx\">\n",
                              w, h);

                buffer.printf("    <image xlink:href=\"crop.png\" "
                              "x=\"0\" y=\"0\" "
                              "width=\"%dpx\" height=\"%dpx\" />\n",
                              w, h);

                for (size_t i = 0; i < path.size() - 1; i++) {
                        check_segment(buffer, mask, path[i], path[i+1], result);
                }
                path.emplace_back(path.back());

                buffer.printf("</svg>\n");
                
                char filename[64];
                snprintf(filename, sizeof(filename), "plant-crossings-%02zu.svg", index);
                session.store_svg(filename, buffer.tostring());
        }

        void go_around(rpp::MemBuffer& buffer, Image& mask, v3 start, v3 end, Path& path)
        {
                int d = (int) kAstarResolution;
                int d2 = d / 2;
                int w = (int) mask.width();
                int h = (int) mask.height();
                float *data = mask.data().data();
                float sum;

                AStar::Generator generator;
                generator.setWorldSize({ (int) (w / d), (int) (h / d) });
                generator.setHeuristic(AStar::Heuristic::euclidean);
                generator.setDiagonalMovement(true);

                for (int y = d2; y < h - d2; y += d) {
                        for (int x = d2; x < w - d2; x += d) {
                                int off = y * w + x;

                                // sum the values over an area of size
                                // dxd and centered on (x,y)
                                sum = 0.0f;
                                for (int j = -d2; j <= d2; j++) {
                                        int offy = off + j * w;
                                        for (int i = -d2; i <= d2; i++) {
                                                sum += data[offy + i];
                                        }
                                }
                                
                                if (sum > 0.0f)
                                        generator.addCollision(AStar::Vec2i((int)(x / d),
                                                                            (int)(y / d)));
                        }
                }

                r_debug("Generate path ... ");
                auto new_path = generator.findPath(
                        { (int) (start.x() / (double) d), (int) (start.y() / (double) d) },
                        { (int) (end.x() / (double) d), (int) (end.y() / (double) d) });
                
                int length = (int) new_path.size();


                
                // TODO
                if (length == 0) {
                        throw std::runtime_error("*** A*: FAILED TO FIND A PATH ***");
                }

                
                
                buffer.printf("    <g>\n");
                buffer.printf("    <path d=\"M %d,%d L %d,%d\" "
                              "fill=\"transparent\" stroke=\"blue\"/>\n",
                              (int) start.x(), (int) start.y(),
                              (int) end.x(), (int) end.y());
                
                
                path.emplace_back(start.x() / (double) mask.width(),
                                  start.y() / (double) mask.width(),
                                  0.0);
                
                // The A* algorithm returns the path from end to
                // start. So we have to iterate backwards. Don't add
                // the last point. This is the start point... and it
                // has already been added above. Don't add the first
                // point (which is the end point) because it will be
                // added by the next segment (to avoid duplicates).
                
                for (int i = length-2; i > 0; i--) {
                        int x = (int) d * new_path[(size_t)i].x + d2;
                        int y = (int) d * new_path[(size_t)i].y + d2;
                        buffer.printf("    <circle cx=\"%dpx\" cy=\"%dpx\" "
                                      "r=\"3px\" fill=\"red\" stroke=\"none\" />\n",
                                      x, y);
                        path.emplace_back((double) x / (double) mask.width(),
                                          (double) y / (double) mask.height(),
                                          0.0);
                        
                }
                buffer.printf("    </g>\n");
        }

        void Pipeline::check_segment(rpp::MemBuffer& buffer,
                                     Image& mask, v3 start, v3 end,
                                     Path& path)
        {
                v3 scale((double) mask.width(), (double) mask.height(), 1.0);
                v3 start_pixels = start * scale;
                v3 end_pixels = end * scale;

                if (segment_crosses_white_area(mask, start_pixels, end_pixels)) {
                        go_around(buffer, mask, start_pixels, end_pixels, path);
                } else {
                        path.emplace_back(start);
                }
        }

}
