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

#ifndef __ROMI_ROVER_OPTIONS_H
#define __ROMI_ROVER_OPTIONS_H

#include "GetOpt.h"

namespace romi {

        static constexpr const char* config_option = "config";
        static constexpr const char* script_option = "script";
        static constexpr const char* soundfont_option = "soundfont";
        static constexpr const char* camera_image_option = "camera-image";
        static constexpr const char* session_directory_option = "session-directory";
        
        extern Option *rover_options;
        extern size_t rover_options_length;

        //void rover_print_usage();
}

#endif // __ROMI_ROVER_OPTIONS_H

