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
#include "turret.h"


gun::gun()
: aim_angle(0.0f)
{}
gun::gun(float aim_angle_)
: aim_angle(aim_angle_)
{}


turret::turret(float aim_speed_)
: aim_speed(aim_speed_)
{}

#include "glm.h"
bool turret::step(float_seconds time)
{
  float max_change = time.count()*aim_speed;
  float gap = rad_diff(aim_angle, target);
  if(std::abs(gap) > max_change)
  {
    if(gap > 0.0f) aim_angle -= max_change;
    else aim_angle += max_change;
    return false;
  }
  else
  {
    aim_angle = target;
    return true;
  }
}
