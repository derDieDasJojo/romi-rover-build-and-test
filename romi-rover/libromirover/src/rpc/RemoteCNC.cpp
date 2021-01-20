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
#include "rpc/RemoteCNC.h"
#include "rpc/MethodsRover.h"

namespace romi {

        bool RemoteCNC::get_range(CNCRange &range)
        {
                r_debug("RemoteCNC::get_range");

                bool success = false;
                JsonCpp result;

                try {
                        if (execute_with_result(MethodsCNC::get_range, result)) {
                                range.init(result);
                                success = true;
                        }
                        
                } catch (JSONError &je) {
                        r_err("RemoteCNC::get_range failed: %s", je.what());
                }

                return success;
        }

        bool RemoteCNC::moveto(double x, double y, double z, double v)
        {
                r_debug("RemoteCNC::moveto");
                
                // TODO: use JSON C++ API
                json_object_t p = json_object_create();
                
                if (x != UNCHANGED)
                        json_object_setnum(p, "x", x);

                if (y != UNCHANGED)
                        json_object_setnum(p, "y", y);
                
                if (z != UNCHANGED)
                        json_object_setnum(p, "z", z);
                
                json_object_setnum(p, "speed", v);
                
                JsonCpp params(p);
                json_unref(p);
                

                return execute_with_params(MethodsCNC::moveto, params);
        }

        bool RemoteCNC::spindle(double speed)
        {
                r_debug("RemoteCNC::spindle");

                // TODO: use JSON C++ API
                json_object_t p = json_object_create();                
                json_object_setnum(p, "speed", speed);
                JsonCpp params(p);
                json_unref(p);

                return execute_with_params(MethodsCNC::spindle, params);
        }
        
        bool RemoteCNC::travel(Path &path, double relative_speed)
        {
                r_debug("RemoteCNC::travel");

                // TODO: use JSON C++ API
                json_object_t parameters = json_object_create();
                json_object_t points = json_array_create();
                for (size_t i = 0; i < path.size(); i++) {
                        json_object_t pt = json_array_create();                
                        json_array_setnum(pt, path[i].x(), 0);
                        json_array_setnum(pt, path[i].y(), 1);
                        json_array_setnum(pt, path[i].z(), 2);
                        json_array_push(points, pt);
                        json_unref(pt);
                }

                json_object_set(parameters, "path", points);
                json_unref(points);
                
                json_object_setnum(parameters, "speed", relative_speed);

                JsonCpp params(parameters);
                json_unref(parameters);

                return execute_with_params(MethodsCNC::travel, params);
        }

        bool RemoteCNC::homing()
        {
                r_debug("RemoteCNC::homing");
                return execute_simple_request(MethodsCNC::homing);
        }

        bool RemoteCNC::pause_activity()
        {
                r_debug("RemoteCNC::pause_activity");
                return execute_simple_request(MethodsActivity::activity_pause);
        }

        bool RemoteCNC::continue_activity()
        {
                r_debug("RemoteCNC::continue_activity");
                return execute_simple_request(MethodsActivity::activity_continue);
        }

        bool RemoteCNC::reset_activity()
        {
                r_debug("RemoteCNC::reset");
                return execute_simple_request(MethodsActivity::activity_reset);
        }

        bool RemoteCNC::power_up()
        {
                r_debug("RemoteCNC::power_up");
                return execute_simple_request(MethodsPowerDevice::power_up);
        }
        
        bool RemoteCNC::power_down()
        {
                r_debug("RemoteCNC::power_down");
                return execute_simple_request(MethodsPowerDevice::power_down);
        }
        
        bool RemoteCNC::stand_by()
        {
                r_debug("RemoteCNC::stand_by");
                return execute_simple_request(MethodsPowerDevice::stand_by);
        }
        
        bool RemoteCNC::wake_up()
        {
                r_debug("RemoteCNC::wake_up");
                return execute_simple_request(MethodsPowerDevice::wake_up);
        }
}
