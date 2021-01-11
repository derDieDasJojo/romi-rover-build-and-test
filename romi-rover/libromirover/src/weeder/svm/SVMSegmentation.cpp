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

#include <stdexcept>
#include "SVMSegmentation.h"

namespace romi {
        
        SVMSegmentation::SVMSegmentation(JsonCpp& params)
        {
                try {
                        set_parameter_a(params["a"]);
                        set_parameter_b(params["b"]);
                } catch (JSONError& je) {
                        r_err("SVMSegmentation: Failed to parse the parameters: %s",
                              je.what());
                        throw std::runtime_error("SVMSegmentation: bad config");
                }
        }
        
        SVMSegmentation::SVMSegmentation(float a[3], float b)
        {
                set_coefficients(a);
                set_intercept(b);
        }

        void SVMSegmentation::set_coefficients(float a[3])
        {
                for (int i = 0; i < 3; i++)
                        _a[i] = a[i];
        }

        float *SVMSegmentation::get_coefficients()
        {
                return _a;
        }
                
        void SVMSegmentation::set_intercept(float b)
        {
                _b = b;
        }
                
        float SVMSegmentation::get_intercept()
        {
                return _b;
        }

        void SVMSegmentation::set_parameter_a(JsonCpp value)
        {
                float a[3];
                a[0] = (float) value.num(0);
                a[1] = (float) value.num(1);
                a[2] = (float) value.num(2);
                set_coefficients(a);
        }
        
        void SVMSegmentation::set_parameter_b(JsonCpp value)
        {
                set_intercept((float) value.num());
        }

        bool SVMSegmentation::segment(IFolder &session, Image &image, Image &mask) 
        {
                if (image.type() != Image::RGB) {
                        r_err("SVMSegmentation::segment: Expected an RGB input image");
                        return false;
                }
                
                mask.init(Image::BW, image.width(), image.height());
                
                int len = image.width() * image.height();
                float *a = image.data();
                float *r = mask.data();
                
                for (int i = 0, j = 0; i < len; i++, j += 3) {
                        float x = (255.0 * a[j] * _a[0]
                                   + 255.0 * a[j+1] * _a[1]
                                   + 255.0 * a[j+2] * _a[2]
                                   + _b);
                        r[i] = (x > 0.0f)? 1.0f : 0.0f;
                }
                return true;
        }
}
