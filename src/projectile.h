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
#ifndef PROJECTILE_H_INCLUDED
#define PROJECTILE_H_INCLUDED


#include "physics.h"


class projectile
{
public:
  class properties
  {
  public:
    properties(float m) : mass(m) {}
    float mass;
  };

  projectile(const properties & t, const glm::vec2 & p, const glm::vec2 & v);
  glm::vec2 position, velocity;

  const properties & type;
};


#include <list>
#include <vector>
class hit_info;
class projectile_world : public bullet_world
{
public:
  projectile_world(bullet_components & components);

  std::list<projectile> projectiles;

  virtual void presubstep(bullet_world::float_seconds substep_time) override;
};


class hit_info
{
public:
  hit_info(const projectile::properties & t, const glm::vec2 & v,
    const glm::vec2 & p, const glm::vec2 & n)
    : type(t), velocity(v), world_point(p), world_normal(n)
  {}

  const projectile::properties & type;
  glm::vec2 velocity;
  glm::vec2 world_point;
  glm::vec2 world_normal;
};


class needs_hit
{
public:
  virtual void hit(const hit_info & info) = 0;
};


class shooter
{
public:
  shooter(std::chrono::steady_clock::duration fire_period_);
  virtual projectile fire() = 0;
  std::chrono::steady_clock::time_point next_fire() const;

  bool wants_fire;
  const std::chrono::steady_clock::duration fire_period;

private:
  std::chrono::steady_clock::time_point next_fire_;
  friend class projectile_world;
};


#endif  // PROJECTILE_H_INCLUDED
