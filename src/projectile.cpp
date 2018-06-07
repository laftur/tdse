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


projectile::projectile(const projectile::properties & t, const glm::vec2 & p,
  const glm::vec2 & v)
: position(p), velocity(v), type(t)
{}


bullet_components projectile_world::components;

projectile_world::projectile_world(const glm::vec2 & boundary_)
: boundary(boundary_),
  bullet_world(components)
{}

void projectile_world::presubstep(bullet_world::float_seconds substep_time)
{
  // Fire all willing and able shooters
  btCollisionObjectArray & array = getCollisionObjectArray();
  for(int i = 0; i < array.size(); ++i)
  {
    // Check if object is a shooter
    shooter * ptr = dynamic_cast<shooter *>( static_cast<body *>(array[i]) );
    if(!ptr) continue;

    // Check will
    if(ptr->wants_fire)
    {
      // Check if it has been long enough since last fire
      auto now = std::chrono::steady_clock::now();
      auto time_until_fire =
        std::chrono::duration_cast<bullet_world::float_seconds>
        (ptr->next_fire_ - now);
      if(time_until_fire.count() < substep_time.count()/2.0f)
      {
        // Fire
        ptr->next_fire_ = now + ptr->fire_period;
        projectiles.emplace_back( ptr->fire() );
      }
    }
  }

  // Step all projectiles
  auto i = projectiles.begin();
  while( i != projectiles.end() )
  {
    const glm::vec2 & pref = i->position;
    // Delete projectiles that have passed boundary
    if(pref.x >= boundary.x || pref.x <= -boundary.x ||
      pref.y >= boundary.y || pref.y <= -boundary.y)
    {
      i = projectiles.erase(i);
      continue;
    }

    // Raycast projectiles still within boundary
    glm::vec2 target = i->position + i->velocity*substep_time.count();
    btVector3 bt_position(i->position.x, i->position.y, 0.0f),
          bt_target(target.x, target.y, 0.0f);
    btCollisionWorld::ClosestRayResultCallback result(bt_position,
      bt_target);
    rayTest(bt_position, bt_target, result);
    if(result.m_collisionObject)
    {
      // Substep callback, so it's safe to modify bodies
      body * victim = static_cast<body *>
        ( const_cast<btCollisionObject *>(result.m_collisionObject) );

      hit_info blam(
        i->type,
        i->velocity,
        glm::vec2( result.m_hitPointWorld.getX(),
          result.m_hitPointWorld.getY() ),
        glm::vec2( result.m_hitNormalWorld.getX(),
          result.m_hitNormalWorld.getY() )
      );
      if( needs_hit * ptr = dynamic_cast<needs_hit *>(victim) )
        ptr->hit(blam);

      i = projectiles.erase(i);
    }
    else
    {
      i->position = target;
      ++i;
      continue;
    }
  }

  bullet_world::presubstep(substep_time);
}


shooter::shooter(std::chrono::steady_clock::duration fire_period_)
: wants_fire(false),
  fire_period(fire_period_),
  next_fire_( std::chrono::steady_clock::now() )
{}
std::chrono::steady_clock::time_point shooter::next_fire() const
{
  return next_fire_;
}
