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
#ifndef SHIP_H_INCLUDED
#define SHIP_H_INCLUDED


#include "projectile.h"
#include "controller.h"
#include <BulletCollision/CollisionShapes/btConeShape.h>
#include <glm/gtc/constants.hpp>
#include <array>


class ship : public body, public needs_presubstep, public needs_hit
{
public:
  static constexpr float max_linear_force = 512.0f;
  static constexpr float max_torque = 64.0f;
  static constexpr float projectile_mass = 0.1f;

  static const std::array<glm::vec2, 3> triangle_vertices;
  static const btConvexHullShape tprism;
  static const btConvex2dShape triangle;
  static const projectile::properties projectile_type;

  ship(const glm::vec2 & position);

  const glm::vec2 & force() const;
  void force(const glm::vec2 & f);
  float torque() const;
  void torque(float t);

  rotation_control rctrl;
  bool rctrl_active;

protected:
  void presubstep(bullet_world & world, float_seconds substep_time) override;
  void hit(const hit_info & info) override;

private:
  bool can_sleep;
  glm::vec2 force_;
  float torque_;
};


#endif  // SHIP_H_INCLUDED
