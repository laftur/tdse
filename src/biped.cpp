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
  // btConvex2dShape never modifies the underlying btCollisionShape
  ( const_cast<btSphereShape *>(&biped::sphere) );

#include <glm/gtc/matrix_transform.hpp>
biped::biped(const glm::vec2 & position)
: actor(glm::pi<float>()*size*size*400.0f, circle, position),
  force_(0.0f, 0.0f)
{
  // Disable rotation
  setAngularFactor(btVector3(0, 0, 0));
  // Ground friction
  setDamping(0.95f, 0.0f);
}

const glm::vec2 & biped::force() const
{
  return force_;
}
void biped::force(const glm::vec2 & force__)
{
  float mag = glm::length(force__);
  if(mag <= max_linear_force) force_ = force__;
  else force_ = force__*(max_linear_force/mag);
}

void biped::presubstep(bullet_world & world, float_seconds substep_time)
{
  if(force_.x != 0.0f || force_.y != 0.0f)
  {
    btRigidBody::activate();
    actor::force(force_);
    setDamping(0.5f, 0.0f);
  }
  else
  {
    // Running bipeds make less ground contact to be fast
    const btVector3 & btvel = btRigidBody::getLinearVelocity();
    float speed_squared = btvel.getX()*btvel.getX() + btvel.getY()*btvel.getY();
    if(speed_squared < 40.0f) setDamping(0.95f, 0.0f);
    else setDamping(0.5f, 0.0f);
  }
}


soldier::soldier(const glm::vec2 & position, std::default_random_engine & prand)
: biped(position),
  shooter( std::chrono::milliseconds(120) ),
  bullet_type(0.008f),
  weapon(8.0f),
  prand_(prand)
{}

std::normal_distribution<float> soldier::normal_dist(0.0f, 0.02f);
projectile soldier::fire()
{
  glm::vec2 velocity(400.0f, 0.0f);
  glm::mat2 direction = mat2_from_angle( weapon.aim_angle()
    + normal_dist(prand_) );
  return projectile(
    bullet_type,
    real_position(),
    direction*velocity
  );
}
