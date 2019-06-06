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
#ifndef __GLM_PRINT_H
#define __GLM_PRINT_H


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class vec2_noexcept : public glm::vec2 {
public:
  vec2_noexcept(float value) noexcept;
  vec2_noexcept(float _x, float _y) noexcept;
  vec2_noexcept(const vec2_noexcept & other) noexcept;
  vec2_noexcept(vec2_noexcept && other) noexcept;
  vec2_noexcept & operator=(const vec2_noexcept & rhs) noexcept;
  vec2_noexcept & operator=(vec2_noexcept && rhs) noexcept;
};

#include <iostream>
std::ostream & operator << (std::ostream & stream, const glm::vec2 & v);
std::ostream & operator << (std::ostream & stream, const glm::vec3 & v);
std::ostream & operator << (std::ostream & stream, const glm::vec4 & v);
std::ostream & operator << (std::ostream & stream, const glm::mat2 & m);
std::ostream & operator << (std::ostream & stream, const glm::mat3 & m);
std::ostream & operator << (std::ostream & stream, const glm::mat4 & m);


glm::mat2 mat2_from_angle(float angle);
float angle_from_mat2(const glm::mat2 & matrix);
glm::mat3 compose_transform( const glm::vec2 & position,
  const glm::mat2 & orientation = glm::mat2(1.0f) );

float rad_diff(float a1, float a2);


#endif
