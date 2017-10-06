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
#include "ship.h"
#include <cmath>


const std::array<glm::vec2, 3> ship::triangle_vertices = {
  glm::vec2( 0.75f,  0.0f),
  glm::vec2(-0.5f,  0.5f),
  glm::vec2(-0.5f, -0.5f),
};
const btConvexHullShape ship::tprism = make_convex_hull
  (ship::triangle_vertices);
const btConvex2dShape ship::triangle
  ( const_cast<btConvexHullShape *>(&ship::tprism) );
const projectile::properties ship::projectile_type(ship::projectile_mass);

ship::ship(const glm::vec2 & position)
: body(64.0f, triangle, position),
  force_(0.0f, 0.0f),
  target_angle(0.0f),
  rctrl(*this, 64.0f)
{
  forceActivationState(DISABLE_DEACTIVATION);
}

const glm::vec2 & ship::force() const
{
  return force_;
}
void ship::force(const glm::vec2 & f)
{
  float mag = glm::length(f);
  if(mag <= max_linear_force) force_ = f;
  else force_ = f*(max_linear_force/mag);
}

void ship::presubstep(bullet_world::float_seconds substep_time)
{
  glm::mat2 ori = real_orientation();

  if(force_.x != 0.0f || force_.y != 0.0f)
  {
    glm::vec2 rotated_force = ori*force_;
    applyCentralForce( btVector3(rotated_force.x, rotated_force.y, 0.0f) );
  }

  applyTorque(
    btVector3( 0.0f, 0.0f, rctrl.torque(target_angle, substep_time) )
  );
}
void ship::hit(const hit_info & info)
{
  // Assume the projectile embedded itself
  glm::vec2 momentum = info.velocity*info.type.mass;
  glm::vec2 local_point = info.world_point - real_position();
  applyImpulse(
    btVector3(momentum.x, momentum.y, 0.0f),
    btVector3(local_point.x, local_point.y, 0.0f)
  );
}
