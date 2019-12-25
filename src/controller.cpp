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

rotation_control::rotation_control(const body & rotating_body,
  float max_torque_)
: inertia( rotating_body.getInvInertiaTensorWorld()[2][2] ),
  subject(rotating_body),
  max_torque(max_torque_),
  target(
    angle_from_mat2( subject.real_orientation() )
  ),
  stop(false)
{}
float rotation_control::torque(float_seconds substep_time) const
{
  float angle = angle_from_mat2( subject.real_orientation() );
  float velocity = subject.getAngularVelocity().getZ();

  // Give up early if we're already stopped
  if(velocity == 0.0f) return 0.0f;
  float max_accel = std::copysign(max_torque*inertia, -velocity);
  float time_to_stop = std::abs(velocity/max_accel);

  if(stop)
  {
    // Boolean stop indicates whether target should be ignored.
    // When ignoring target, objective is to stop rotation ASAP.
    if( time_to_stop >= substep_time.count() )
      return std::copysign(max_torque, -velocity);
    else
      return -velocity/time_to_stop*inertia;
  }
  else
  {
    /*
     * Approach:
     * stop_point is a future angle where we are projected to achieve zero
     * velocity.
     * Adjust the stop point by accelerating until stop_point == target.
     * When stop_point == target, decelerate until velocity == 0 and
     * angle == target.
     */
    float stop_point = angle + velocity*time_to_stop +
      max_accel*time_to_stop*time_to_stop/2.0f;
    float stop_diff = rad_diff(target, stop_point);

    return std::copysign(max_torque, -velocity) + 2.0f*stop_diff /
      ( inertia*substep_time.count()*substep_time.count() );
  }
}
