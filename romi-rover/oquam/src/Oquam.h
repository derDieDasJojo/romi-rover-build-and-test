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
#ifndef __ROMI_OQUAM_H
#define __ROMI_OQUAM_H

#include <r.h>
#include <mutex>
#include "v.h"
#include "CNC.h"
#include "CNCController.h"
#include "IFileCabinet.h"
#include "Script.h" 

namespace romi {

        using SynchonizedCodeBlock = std::lock_guard<std::mutex>;
        
        class Oquam : public CNC
        {
        public:
                CNCController& _controller;
                IFileCabinet *_file_cabinet;
                std::mutex _m;

                CNCRange _range;
                double _vmax[3]; // in m/s
                double _amax[3]; // in m/s²

                // The maximum deviation allowed when computed a
                // continuous path, in m.
                double _path_max_deviation;
                double _scale_meters_to_steps[3];
                double _path_slice_interval;
                int _script_count;
                
        public:
                
                Oquam(CNCController& controller, CNCRange& range,
                      const double *vmax, const double *amax,
                      const double *scale_meters_to_steps, 
                      double path_max_deviation,
                      double path_slice_interval);
                
                virtual ~Oquam() = default;

                void set_file_cabinet(IFileCabinet *cabinet) {
                        _file_cabinet = cabinet;
                }
                

                // See CNC.h for more info
                bool moveto(double x, double y, double z,
                            double relative_speed = 0.1) override;
                bool travel(Path &path, double relative_speed = 0.1) override;
                bool spindle(double speed) override;
                bool homing() override;
                bool get_range(CNCRange &range) override;

                bool stop_execution() override;
                bool continue_execution() override;
                bool reset() override;

        protected:
                
                bool moveto_synchronized(double x, double y, double z, double rel_speed);
                void do_moveto(double x, double y, double z, double rel_speed);
                bool travel_synchronized(Path &path, double relative_speed);
                void do_travel(Path &path, double relative_speed);
                void convert_path_to_script(Path &path, double speed, Script& script); 
                void convert_script(Script& script, double *vmax);
                void store_script(Script& script);
                void check_script(Script& script, double *vmax);
                void execute_script(Script& script);
                void execute_move(Section& section, int32_t *pos_steps);
                void wait_end_of_script(Script& script); 
                double script_duration(Script& script);
                bool get_position(double *position); 
                bool get_position(int32_t *position); 
                void assert_get_position(double *position); 
                void assert_relative_speed(double relative_speed); 
                void assert_in_range(double x, double y, double z);
                void compute_steps_and_millis(double x, double y, double z,
                                              double rel_speed,
                                              int16_t *results);
                bool is_zero(int16_t *params);
                double get_duration(int16_t *params);
                void move_and_synchronize(int16_t *params);
                void assert_move(int16_t *params);
                void assert_synchronize(double timeout);
        };
}

#endif // __ROMI_OQUAM_H
