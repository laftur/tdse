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
warship::weapon::weapon(const glm::vec2 & mount_point_,
                        const projectile::properties & bullet__)
: periodic( std::chrono::milliseconds(120) ),
  mount_point(mount_point_),
  enabled(false),
  bullet_(&bullet__)
{}
const projectile::properties & warship::weapon::bullet() const
{
  return *bullet_;
}
void warship::weapon::bullet(const projectile::properties & type)
{
  bullet_ = &type;
}


warship::platform::platform(const glm::vec2 & offset_, float offset_angle_)
: offset(offset_), offset_angle(offset_angle_)
{}
void warship::platform::fire(bool enable)
{
  for(auto i = weapons.begin(); i != weapons.end(); ++i)
    i->enabled = enable;
  for(auto i = subplatforms.begin(); i != subplatforms.end(); ++i)
    i->fire(enable);
}


warship::warship(const glm::mat3 & transform, std::default_random_engine & prand)
: ship(transform),
  weapon_tree(glm::vec2(0.0f, 0.0f), 0.0f),
  prand_(prand)
{}

void warship::step(const glm::vec2 & offset,
                   const glm::mat2 & offset_orientation,
                   platform & tree, bullet_world & world, float_seconds time)
{
  glm::vec2 velocity;
  {
    const btVector3 & btvel = btRigidBody::getLinearVelocity();
    velocity.x = btvel.getX();
    velocity.y = btvel.getY();
  }

  glm::vec2 tree_position = tree.offset + offset;
  glm::mat2 tree_orientation =
    mat2_from_angle(tree.offset_angle)*offset_orientation;

  for(auto wpn = tree.weapons.begin(); wpn != tree.weapons.end(); ++wpn)
  {
    wpn->step(time);
    while( wpn->ready() )
    {
      if(wpn->enabled)
      {
        float_seconds remainder = wpn->trigger();
        glm::mat2 orientation =
          mat2_from_angle( normal_dist(prand_) ) * tree_orientation;

        // Create a new projectile
        projectiles.emplace_back(
          wpn->bullet(),
          offset_orientation*wpn->mount_point + tree_position,
          orientation*glm::vec2(400.0f, 0.0f) + velocity
        );
        // Fire period usually elapses before the end of the step,
        // so step the new projectile ahead by the remaining time
        if( projectiles.back().step(world, remainder) ) projectiles.pop_back();
      }
      else
      {
        wpn->reset();
        break;
      }
    }
  }
  for(auto i = tree.subplatforms.begin(); i != tree.subplatforms.end(); ++i)
    step(tree_position, tree_orientation, *i, world, time);
}
void warship::presubstep(bullet_world & world, float_seconds substep_time)
{
  ship::presubstep(world, substep_time);

  // Step all projectiles
  for(auto i = projectiles.begin(); i != projectiles.end(); )
  {
    // Erase projectiles that collide or expire
    if( i->step(world, substep_time) )
      i = projectiles.erase(i);
    else ++i;
  }

  // Fire all weapons and step subplatforms
  step(real_position(), real_orientation(), weapon_tree, world, substep_time);
}

std::normal_distribution<float> warship::normal_dist(0.0f, 0.02f);
