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
#ifndef BIPED_H_INCLUDED
#define BIPED_H_INCLUDED


#include "projectile.h"
#include <BulletCollision/CollisionShapes/btSphereShape.h>


class biped : public actor, public needs_presubstep
{
public:
  static constexpr float size = 0.25f;
  static constexpr float max_linear_force = 400.0f;

  static const btSphereShape sphere;
  static const btConvex2dShape circle;

  biped(const glm::vec2 & position);

  const glm::vec2 & force() const;
  void force(const glm::vec2 & f);

protected:
  void presubstep(bullet_world & world, float_seconds substep_time) override;

private:
  glm::vec2 force_;
};


#include <random>
#include "turret.h"
#include "shooter.h"
class soldier : public biped, public shooter
{
public:
  soldier(const glm::vec2 & position, std::default_random_engine & prand);
  soldier(const soldier &) = delete;
  void operator=(const soldier &) = delete;

  projectile::properties bullet_type;
  turret weapon;

private:
  std::default_random_engine & prand_;
  static std::normal_distribution<float> normal_dist;

protected:
  projectile fire() override;
};


#endif  // BIPED_H_INCLUDED
