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
#ifndef __CAMERA_H
#define __CAMERA_H


#include "glm.h"
class camera
{
public:
  camera(const glm::vec2 & _position, float _orientation,
    float _magnification);
  camera(const glm::vec2 & _position, const glm::mat2 & _orientation,
    float _magnification);
  camera(const glm::mat3 & _transform, float _magnification);
  camera(const camera &) = default;
  camera & operator = (const camera &) = default;

  glm::vec2 position;
  glm::mat2 orientation;
  float magnification;

  glm::mat3 view() const;
  glm::mat3 transform() const;
  void transform(const glm::mat3 & t);
};



#include "glm.h"
#include <chrono>
class kinematic_camera : public camera
{
public:
  kinematic_camera(const glm::vec2 & _position, float _orientation,
    float _magnification);
  kinematic_camera(const glm::vec2 & _position,
    const glm::mat2 & _orientation, float _magnification);
  kinematic_camera(const glm::mat3 & _transform, float _magnification);

  bool step(std::chrono::duration<float, std::ratio<1> > time);

  glm::vec2 velocity;
  glm::vec2 relative_velocity;
  float angular_velocity;
  float magnification_velocity;
};


#endif
