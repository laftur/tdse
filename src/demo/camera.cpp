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
#include "camera.h"


#include <glm/gtc/matrix_transform.hpp>
camera::camera(const glm::vec2 & _position, float _orientation,
  float _magnification)
: position(_position),
  orientation(
    glm::mat2(
      glm::rotate(
        glm::mat4(1.0),
        _orientation,
        glm::vec3(0.0, 0.0, 1.0)
      )
    )
  ),
  magnification(_magnification)
{}
camera::camera(const glm::vec2 & _position, const glm::mat2 & _orientation,
  float _magnification)
: position(_position),
  orientation(_orientation),
  magnification(_magnification)
{}
camera::camera(const glm::mat3 & _transform, float _magnification)
: position(glm::vec2(_transform[2])),
  orientation(glm::mat2(_transform)),
  magnification(_magnification)
{}

glm::mat3 camera::view() const
{
  glm::mat2 m(inverse(orientation));
  return glm::mat3(
    glm::vec3(m[0]*magnification, 0.0f),
    glm::vec3(m[1]*magnification, 0.0f),
    glm::vec3(-(m*position)*glm::vec2(magnification), 1.0f)
  );
}
glm::mat3 camera::transform() const
{
  return glm::mat3(
    glm::vec3(orientation[0], 0.0f),
    glm::vec3(orientation[1], 0.0f),
    glm::vec3(position, 1.0f)
  );
}
void camera::transform(const glm::mat3 & t)
{
  position = glm::vec2(t[2]);
  orientation = glm::mat2(t);
}


kinematic_camera::kinematic_camera(const glm::vec2 & _position,
  float _orientation, float _magnification)
: camera(_position, _orientation, _magnification),
  velocity(0.0),
  relative_velocity(0.0),
  angular_velocity(0.0),
  magnification_velocity(0.0)
{}
kinematic_camera::kinematic_camera(const glm::vec2 & _position,
  const glm::mat2 & _orientation, float _magnification)
: camera(_position, _orientation, _magnification),
  velocity(0.0),
  relative_velocity(0.0),
  angular_velocity(0.0),
  magnification_velocity(0.0)
{}
kinematic_camera::kinematic_camera(const glm::mat3 & _transform,
  float _magnification)
: camera(_transform, _magnification),
  velocity(0.0),
  relative_velocity(0.0),
  angular_velocity(0.0),
  magnification_velocity(0.0)
{}

#include <glm/gtc/matrix_transform.hpp>
bool kinematic_camera::step(std::chrono::duration<float, std::ratio<1> > time)
{
  bool moved = false;

  if(velocity.x != 0.0 || velocity.y != 0.0)
  {
    position += velocity*time.count();
    moved = true;
  }
  if(relative_velocity.x != 0.0 || relative_velocity.y != 0.0)
  {
    position += orientation*(relative_velocity/magnification)*time.count();
    moved = true;
  }
  if(angular_velocity != 0.0)
  {
    orientation *= glm::mat2(
      glm::rotate(
        glm::mat4(1.0),
        angular_velocity*time.count(),
        glm::vec3(0.0, 0.0, 1.0)
      )
    );
    moved = true;
  }
  if(magnification_velocity != 0.0)
  {
    magnification *= glm::pow(2.0, magnification_velocity*time.count());
    moved = true;
  }

  return moved;
}
