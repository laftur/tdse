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
#include "glm.h"


vec2_noexcept::vec2_noexcept(float value) noexcept
: glm::vec2(value)
{}
vec2_noexcept::vec2_noexcept(float _x, float _y) noexcept
: glm::vec2(_x, _y)
{}
vec2_noexcept::vec2_noexcept(const vec2_noexcept & other) noexcept
: glm::vec2(other)
{}
#include <utility>
vec2_noexcept::vec2_noexcept(vec2_noexcept && other) noexcept
: glm::vec2(std::move(other))
{}
vec2_noexcept & vec2_noexcept::operator=(const vec2_noexcept & rhs) noexcept {
  glm::vec2::operator=(rhs);
  return *this;
}
vec2_noexcept & vec2_noexcept::operator=(vec2_noexcept && rhs) noexcept {
  glm::vec2::operator=(std::move(rhs));
  return *this;
}


#include <iomanip>
std::ostream & operator << (std::ostream & stream, const glm::vec2 & v) {
  std::streamsize prev = std::cout.precision(3);

  stream << '('
    << std::setw(9) << v.x << ", "
    << std::setw(9) << v.y << ')';

  std::cout.precision(prev);
  return stream;
}
std::ostream & operator << (std::ostream & stream, const glm::vec3 & v) {
  std::streamsize prev = std::cout.precision(3);

  stream << '('
    << std::setw(9) << v.x << ", "
    << std::setw(9) << v.y << ", "
    << std::setw(9) << v.z << ')';

  std::cout.precision(prev);
  return stream;
}
std::ostream & operator << (std::ostream & stream, const glm::vec4 & v) {
  std::streamsize prev = std::cout.precision(3);

  stream << '('
    << std::setw(9) << v.x << ", "
    << std::setw(9) << v.y << ", "
    << std::setw(9) << v.z << ", "
    << std::setw(9) << v.w << ')';

  std::cout.precision(prev);
  return stream;
}
std::ostream & operator << (std::ostream & stream, const glm::mat2 & m) {
  glm::mat2 mt = glm::transpose(m);
  stream << '|' << mt[0] << "|\n" 
    << '|' << mt[1] << "|\n";
  return stream;
}
std::ostream & operator << (std::ostream & stream, const glm::mat3 & m) {
  glm::mat3 mt = glm::transpose(m);
  stream << '|' << mt[0] << "|\n" 
    << '|' << mt[1] << "|\n"
    << '|' << mt[2] << "|\n";
  return stream;
}
std::ostream & operator << (std::ostream & stream, const glm::mat4 & m) {
  glm::mat4 mt = glm::transpose(m);
  stream << '|' << mt[0] << "|\n" 
    << '|' << mt[1] << "|\n" 
    << '|' << mt[2] << "|\n" 
    << '|' << mt[3] << "|\n";
  return stream;
}


#include <glm/gtc/matrix_transform.hpp>
glm::mat2 mat2_from_angle(float angle)
{
  return glm::mat2(glm::rotate( glm::mat4(1.0f), angle,
    glm::vec3(0.0f, 0.0f, 1.0f) ));
}
#include <cmath>
float angle_from_mat2(const glm::mat2 & matrix)
{
  return atan2(matrix[0][1], matrix[0][0]);
}
glm::mat3 compose_transform(const glm::vec2 & position,
  const glm::mat2 & orientation)
{
  return glm::mat3( glm::vec3(orientation[0], 0.0f),
    glm::vec3(orientation[1], 0.0f),
    glm::vec3(position, 1.0f) );
}

float rad_diff(float a, float b)
{
  float result = std::fmod( a - b, glm::two_pi<float>() );
  if( std::abs(result) <= glm::pi<float>() ) return result;
  else return std::fmod( b - a, glm::two_pi<float>() );
}
