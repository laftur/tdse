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
#include "controller.h"

rot_ctrl::rot_ctrl(float _max_torque)
: max_torque(_max_torque) {}
float rot_ctrl::calc_torque(const body & b, float target,
                            float_seconds substep_time) const
{
  float angle = angle_from_mat2( b.real_orientation() );
  float v = b.getAngularVelocity().getZ();
  float ri = b.getInvInertiaTensorWorld()[2][2];
  float a = std::copysign(max_torque*ri, -v);
  float stop_time = std::abs(v/a);
  float stop_angle = angle + v*stop_time + a*stop_time*stop_time/2.0f;
  float stop_diff = rad_diff(target, stop_angle);

  a = stop_diff / ( ri*substep_time.count()*substep_time.count() );
  if(std::abs(a) > max_torque)
    return std::copysign(max_torque, a);
  else
    return a;
}
