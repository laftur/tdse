/*
Copyright (C) 2016 Jeremy Starnes

This file is part of TDSE.

TDSE is free software: you can redistribute it and/or modify it under the terms
of the GNU Affero General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

TDSE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along
with TDSE; see the file COPYING. If not, see <http://www.gnu.org/licenses/agpl>
*/
#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "physics.h"

class rot_ctrl
{
public:
  const float max_torque;

  rot_ctrl(float _max_torque);
  float calc_torque(const body & b, float target,
    bullet_world::float_seconds substep_time) const;
};


#endif  // CONTROLLER_H_INCLUDED
