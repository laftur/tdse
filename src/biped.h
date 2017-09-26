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


class biped : public body, public needs_presubstep, public needs_hit
{
public:
  static constexpr float size = 1.0f;
  static constexpr float max_linear_force = 128.0f;

  static const btSphereShape sphere;
  static const btConvex2dShape circle;

  biped(const glm::vec2 & position);

  const glm::vec2 & force() const;
  void force(const glm::vec2 & f);

protected:
  virtual void presubstep(bullet_world::float_seconds substep_time) override;
  virtual void hit(const hit_info & info) override;

private:
  bool can_sleep;
  glm::vec2 _force;
};


#endif  // BIPED_H_INCLUDED
