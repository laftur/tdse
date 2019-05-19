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
#include "projectile.h"


projectile::properties::properties(float mass_)
: mass(mass_)
{}

projectile::projectile(const properties & type_,
                       const glm::vec2 & position_,
                       const glm::vec2 & velocity_)
: position(position__), velocity(velocity__), type(type_),
  position__(position_), velocity__(velocity_)
{}
projectile::projectile(const projectile & other)
: position(position__), velocity(velocity__), type(other.type),
  position__(other.position__), velocity__(other.velocity__)
{}

bool projectile::step(btCollisionWorld & world, float_seconds time)
{
  // Calculate next position after step
  glm::vec2 target = position__ + velocity__*time.count();
  btVector3 bt_position(position__.x, position__.y, 0.0f),
            bt_target(target.x, target.y, 0.0f);
  position__ = target;

  // Raycast to find first collision
  btCollisionWorld::ClosestRayResultCallback result(bt_position, bt_target);
  world.rayTest(bt_position, bt_target, result);
  if(result.m_collisionObject)
  {
    // Collision happened

    body * victim = static_cast<body *>
      // It's safe to modify btCollisionObjects in between substeps
      ( const_cast<btCollisionObject *>(result.m_collisionObject) );

    // If needed, notify with collision information
    if( needs_hit * ptr = dynamic_cast<needs_hit *>(victim) )
      ptr->hit( hit_info(
        type,
        velocity__,
        glm::vec2( result.m_hitPointWorld.getX(),
          result.m_hitPointWorld.getY() ),
        glm::vec2( result.m_hitNormalWorld.getX(),
          result.m_hitNormalWorld.getY() )
      ) );

    return true;
  }

  // No collision detected
  return false;
}


hit_info::hit_info(const projectile::properties & t, const glm::vec2 & v,
                   const glm::vec2 & p, const glm::vec2 & n)
: type(t), velocity(v), world_point(p), world_normal(n)
{}


periodic::periodic(float_seconds period__)
: enabled(false),
  cooldown(cooldown_),
  cooldown_(0.0f),
  period_(period__)
{}
periodic::periodic(const periodic & other)
: enabled(false),
  cooldown(cooldown_),
  cooldown_(other.cooldown_),
  period_(other.period_)
{}
periodic & periodic::operator=(const periodic & rhs)
{
  cooldown_ = rhs.cooldown_;
  period_ = rhs.period_;

  return *this;
}

float_seconds periodic::period() const
{
  return period_;
}
void periodic::period(float_seconds period__)
{
  if(period__ > bullet_world::fixed_substep)
    period_ = period__;
  else
    period_ = bullet_world::fixed_substep;
}

void periodic::presubstep(bullet_world & world, float_seconds substep_time)
{
  cooldown_ -= substep_time;
  if(cooldown_.count() <= 0.0f)
  {
    // Period elapsed
    if(enabled)
    {
      trigger(world, -cooldown_);

      // Cool down until period has elapsed
      cooldown_ += period_;
    }
    else cooldown_ = float_seconds(0.0f);
  }
}


shooter::shooter(float_seconds fire_period)
: periodic(fire_period)
{}
void shooter::presubstep(bullet_world & world, float_seconds substep_time)
{
  // Projectiles outside boundary (square half-extents) are deleted
  static constexpr float boundary = 1000.0f;

  // Step all projectiles
  for(auto i = projectiles.begin(); i != projectiles.end(); )
  {
    // Erase projectiles that pass boundary or collide
    auto position = i->position;
    if( position.x >  boundary ||
        position.x < -boundary ||
        position.y >  boundary ||
        position.y < -boundary ||
        i->step(world, substep_time) )
      i = projectiles.erase(i);  // Erase returns next projectile.
    else ++i;
  }

  periodic::presubstep(world, substep_time);
}

void shooter::trigger(bullet_world & world, float_seconds remainder)
{
  // Create a new projectile
  projectiles.emplace_back( fire() );
  // Fire period usually elapses before the end of the substep,
  // so step the new projectile ahead with the remaining substep time
  if( projectiles.back().step(world, remainder) ) projectiles.pop_back();
}


actor::actor(float mass,
             const btCollisionShape & shape,
             const glm::vec2 & position)
: body(mass, shape, position)
{}
void actor::force(const glm::vec2 & force_)
{
  btRigidBody::applyCentralForce( btVector3(force_.x, force_.y, 0.0f) );
}
void actor::torque(float torque_)
{
  btRigidBody::applyTorque( btVector3(0.0f, 0.0f, torque_) );
}

void actor::hit(const hit_info & info)
{
  btRigidBody::activate();

  // Assume the projectile embedded itself
  glm::vec2 momentum = info.velocity*info.type.mass;
  glm::vec2 local_point = info.world_point - real_position();
  btRigidBody::applyImpulse(
    btVector3(momentum.x, momentum.y, 0.0f),
    btVector3(local_point.x, local_point.y, 0.0f)
  );
}
