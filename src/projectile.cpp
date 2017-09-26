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


projectile_world::projectile_world(bullet_components & components)
: bullet_world(components)
{}

void projectile_world::presubstep(bullet_world::float_seconds time)
{
  hits.clear();

  auto i = projectiles.begin();
  while( i != projectiles.end() )
  {
    glm::vec2 target = i->position + i->velocity*time.count();
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

      hits.emplace_back(
        i->type,
        i->velocity,
        glm::vec2( result.m_hitPointWorld.getX(),
          result.m_hitPointWorld.getY() ),
        glm::vec2( result.m_hitNormalWorld.getX(),
          result.m_hitNormalWorld.getY() )
      );
      if( needs_hit * ptr = dynamic_cast<needs_hit *>(victim) )
        ptr->hit( hit_info(hits.back()) );

      i = projectiles.erase(i);
    }
    else
    {
      i->position = target;
      ++i;
    }
  }

  bullet_world::presubstep(time);
}
