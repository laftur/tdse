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
#include "biped.h"
#include <cmath>


const btSphereShape biped::sphere(biped::size);
const btConvex2dShape biped::circle
  ( const_cast<btSphereShape *>(&biped::sphere) );

#include <glm/gtc/matrix_transform.hpp>
biped::biped(const glm::vec2 & position)
  : body(glm::pi<float>()*size*size*2.0f, circle, position),
  _force(0.0f, 0.0f)
{
  // Disable rotation
  setAngularFactor(btVector3(0, 0, 0));
  // Ground friction
  setDamping(0.95f, 0.0f);
}

const glm::vec2 & biped::force() const
{
  return _force;
}
void biped::force(const glm::vec2 & f)
{
  float mag = glm::length(f);
  if(mag <= max_linear_force) _force = f;
  else _force = f*(max_linear_force/mag);
}

void biped::presubstep(bullet_world::float_seconds substep_time)
{
  if(_force.x != 0.0f || _force.y != 0.0f)
  {
    btRigidBody::activate();
    applyCentralForce( btVector3(_force.x, _force.y, 0.0f) );
    setDamping(0.5f, 0.0f);
  }
  else
  {
    // Bipeds tend to stop suddenly
    const btVector3 & btvel = getLinearVelocity();
    float speed_squared = btvel.getX()*btvel.getX() + btvel.getY()*btvel.getY();
    if(speed_squared < 40.0f) setDamping(0.95f, 0.0f);
    else setDamping(0.5f, 0.0f);
  }
}
void biped::hit(const hit_info & info)
{
  btRigidBody::activate();

  // Assume the projectile embedded itself
  glm::vec2 momentum = info.velocity*info.type.mass;
  glm::vec2 local_point = info.world_point - real_position();
  applyImpulse(
    btVector3(momentum.x, momentum.y, 0.0f),
    btVector3(local_point.x, local_point.y, 0.0f)
  );
}
