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
#ifndef __ROMI_NAVIGATION_H
#define __ROMI_NAVIGATION_H

#include "INavigation.h" 
#include "SynchronizedCodeBlock.h" 

namespace romi {
        
        class Navigation : public INavigation
        {
        protected:

                enum { ROVER_MOVEAT_CAPABLE, ROVER_MOVING };
                
                IMotorDriver &_driver;
                mutex_t *_mutex;
                int _status;

                bool do_move(double distance, double speed) {
                        return false;
                }
                                        
        public:
                Navigation(IMotorDriver &driver) :
                        _driver(driver), _status(ROVER_MOVEAT_CAPABLE) {
                        _mutex = new_mutex();
                }
                
                virtual ~Navigation() override {
                        if (_mutex)
                                delete_mutex(_mutex);
                }

                bool moveat(double left, double right) override {
                        SynchronizedCodeBlock sync(_mutex);
                        bool success = false;
                        if (_status == ROVER_MOVEAT_CAPABLE) 
                                success = _driver.moveat(left, right);
                        else
                                r_warn("Navigation::moveat: still moving");
                        return success;
                }

                bool move(double distance, double speed) override {
                        bool success = false;
                        {
                                SynchronizedCodeBlock sync(_mutex);
                                if (_status == ROVER_MOVEAT_CAPABLE) {
                                        _status = ROVER_MOVING;
                                } else {
                                        r_warn("Navigation::move: already moving");
                                }
                        }
                        
                        if (_status == ROVER_MOVING)
                                success = do_move(distance, speed);
                        
                        _status = ROVER_MOVEAT_CAPABLE;
                        
                        return success;
                        
                }
        };
}

#endif // __ROMI_NAVIGATION_H
