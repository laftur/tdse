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


projectile::properties::properties(float mass_, float range)
: mass(mass_), range_squared(range*range)
{}

projectile::projectile(const properties & type_,
                       const glm::vec2 & position_,
                       const glm::vec2 & velocity_)
: type(type_), position__(position_), velocity__(velocity_), origin(position__)
{}

bool projectile::step(btCollisionWorld & world, float_seconds time)
{
  // Return early if we're too far from the origin
  auto diff = position__ - origin;
  if(diff.x*diff.x + diff.y*diff.y > type.range_squared)
    return true;

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
const glm::vec2 & projectile::position() const
{
  return position__;
}
const glm::vec2 & projectile::velocity() const
{
  return velocity__;
}


hit_info::hit_info(const projectile::properties & t, const glm::vec2 & v,
                   const glm::vec2 & p, const glm::vec2 & n)
: type(t), velocity(v), world_point(p), world_normal(n)
{}


actor::actor(float mass,
             const btCollisionShape & shape,
             const glm::mat3 & transform)
: body(mass, shape, transform)
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
