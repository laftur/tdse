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


const std::array<glm::vec2, 3> ship::triangle_vertices = {
  glm::vec2( 0.75f,  0.0f),
  glm::vec2(-0.5f,  0.5f),
  glm::vec2(-0.5f, -0.5f),
};
const btConvexHullShape ship::tprism = make_convex_hull
  (ship::triangle_vertices);
const btConvex2dShape ship::triangle
  ( const_cast<btConvexHullShape *>(&ship::tprism) );

ship::ship(const glm::mat3 & transform)
: actor(64.0f, triangle, transform),
  rctrl(*this, max_torque),
  rctrl_active(false),
  force_(0.0f, 0.0f),
  torque_(0.0f)
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
float ship::torque() const
{
  return torque_;
}
#include <cmath>
void ship::torque(float t)
{
  if(std::abs(t) <= max_torque) torque_ = t;
  else torque_ = std::copysign(max_torque, t);
}

void ship::presubstep(bullet_world & world, float_seconds substep_time)
{
  if(force_.x != 0.0f || force_.y != 0.0f)
    actor::force(real_orientation()*force_);

  if(rctrl_active) actor::torque( rctrl.torque(substep_time) );
  else actor::torque(torque_);
}


#include <chrono>
warship::weapon::weapon(const warship & wielder_,
                        const gun & aim_,
                        const glm::vec2 & mount_point_,
                        const projectile::properties & bullet_type_)
: shooter( std::chrono::milliseconds(120) ),
  wielder(wielder_),
  aim(aim_),
  mount_point(mount_point_),
  bullet_type(bullet_type_)
{}
projectile warship::weapon::fire()
{
  glm::vec2 velocity(400.0f, 0.0f);
  glm::mat2 direction = mat2_from_angle(
    aim.aim_angle +
    normal_dist(wielder.prand_)
  ) * wielder.real_orientation();

  return projectile(
    bullet_type,
    wielder.real_position(),
    direction*velocity
  );
}
std::normal_distribution<float> warship::weapon::normal_dist(0.0f, 0.02f);

warship::warship(const glm::mat3 & transform, std::default_random_engine & prand)
: ship(transform),
  prand_(prand)
{}

void warship::add_weapon(const gun & aim_,
                         const glm::vec2 & mount_point_,
                         const projectile::properties & bullet_type_)
{
  weapons.emplace_back(*this, aim_, mount_point_, bullet_type_);
}
void warship::add_all(bullet_world & world)
{
  for(auto i = weapons.begin(); i != weapons.end(); ++i)
    world.add_callback(*i);
}
void warship::remove_all(bullet_world & world)
{
  for(auto i = weapons.begin(); i != weapons.end(); ++i)
    world.remove_callback(*i);
}
#include <iostream>
void warship::fire(bool enable)
{
  for(auto i = weapons.begin(); i != weapons.end(); ++i)
    i->enabled = enable;
}
