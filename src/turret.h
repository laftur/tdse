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
#ifndef TURRET_H_INCLUDED
#define TURRET_H_INCLUDED


class gun
{
public:
  gun();
  gun(float aim_angle_);

  float aim_angle;
};


#include "physics.h"
class turret : public gun
{
public:
  turret(float aim_speed_);

  const float aim_speed;

  float target;
  // Returns true when aimed at target
  bool step(float_seconds time);
};


#endif  // TURRET_H_INCLUDED
