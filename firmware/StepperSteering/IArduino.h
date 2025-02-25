/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  Azhoo is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef _STEERING_I_ARDUINO_H
#define _STEERING_I_ARDUINO_H

#include "IEncoder.h"

class IArduino
{
public:
        virtual ~IArduino() = default;
        virtual IEncoder& left_encoder() = 0;
        virtual IEncoder& right_encoder() = 0;
        virtual uint32_t milliseconds() = 0;

        virtual void init_encoders(uint16_t encoder_steps,
                                   int8_t left_increment,
                                   int8_t right_increment) = 0;
};

#endif // _STEERING_I_ARDUINO_H
