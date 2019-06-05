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
#ifndef SHAPE_RENDERER_H_INCLUDED
#define SHAPE_RENDERER_H_INCLUDED


#include "graphics.h"


class segment
{
public:
  glm::vec2 start, end;
  segment(const glm::vec2 & s, const glm::vec2 & e) : start(s), end(e) {}
  segment(const glm::vec2 & s) : start(s) {}
  segment() {}
};
class shape
{
public:
  template<typename T, typename U> shape(const T & vertices,
    const U & indices, GLenum mode);

private:
  GLuint vertex_buffer, index_buffer;
  const GLsizei _indices;
  GLenum _mode;
  friend class shape_renderer;
};

template<typename T, typename U> shape::shape(const T & vertices,
  const U & indices, GLenum mode)
: _indices( indices.size() ),
  _mode(mode)
{
  glGenBuffers(1, &vertex_buffer);
  glGenBuffers(1, &index_buffer);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec2),
    vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLushort),
    indices.data(), GL_STATIC_DRAW);
}


class shape_renderer : public renderer
{
public:
  shape_renderer(window & w);
  ~shape_renderer();

  template<typename T> void render(const T & transforms, const shape & m);
  template<typename T> void render(const T & segments);
  void render(const glm::mat3 & transform, const shape & m);
  void render(const segment & s);

private:
  GLuint vertex_attribute_index;
  GLuint segment_vertex_buffer;
};

shape_renderer::shape_renderer(window & w)
: renderer(w),
  vertex_attribute_index(gpu_program.get_attribute("vertex"))
{
  glGenBuffers(1, &segment_vertex_buffer);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glClearColor(0.0, 0.0, 0.0, 0.0);
}
shape_renderer::~shape_renderer()
{
  glDeleteBuffers(1, &segment_vertex_buffer);
}

template<typename T> void shape_renderer::render(const T & transforms,
  const shape & m)
{
  glBindBuffer(GL_ARRAY_BUFFER, m.vertex_buffer);
  glEnableVertexAttribArray(vertex_attribute_index);
  glVertexAttribPointer(
    vertex_attribute_index,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,
    (GLvoid *)0
  );
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.index_buffer);

  for(auto i = transforms.begin(); i != transforms.end(); ++i)
  {
    model(*i);
    update();

    glDrawElements(
      m._mode,
      m._indices,
      GL_UNSIGNED_SHORT,
      (GLvoid *)0
    );
  }
}
template<typename T> void shape_renderer::render(const T & segments)
{
  model( glm::mat3(1.0f) );
  update();

  glBindBuffer(GL_ARRAY_BUFFER, segment_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, segments.size()*sizeof(segment),
    segments.data(), GL_STREAM_DRAW);
  glEnableVertexAttribArray(vertex_attribute_index);
  glVertexAttribPointer(
    vertex_attribute_index,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,
    (GLvoid *)0
  );

  glDrawArrays(
    GL_LINES,
    0,
    segments.size()*2
  );
}
void shape_renderer::render(const glm::mat3 & transform,
  const shape & m)
{
  render(std::vector<glm::mat3>(1, transform), m);
}
void shape_renderer::render(const segment & s)
{
  render(std::vector<segment>(1, s));
}


#endif  // SHAPE_RENDERER_H_INCLUDED
