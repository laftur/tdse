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
#include "graphics.h"


// GLEW multi-context support
#ifdef GLEW_MX
GLEWContext * glewGetContext()
{
  if(glew_context::current == nullptr) throw std::runtime_error("no GLEW context is current");
  return &glew_context::current.context;
}
#endif

glew_context::glew_context()
{
  try
  {
    set_rollback();
    make_current();

    if(glewInit() != GLEW_OK) throw std::runtime_error("glewInit failed");
    // Discard any gl error caused by glewInit()
    while(glGetError() != GL_NO_ERROR);
  }
  catch(const std::exception & e)
  {
    rollback();
    throw e;
  }
}
glew_context::~glew_context()
{
  if(current == this) current = nullptr;
}

void glew_context::make_current()
{
  current = this;
}

void glew_context::set_rollback()
{
  _rollback = current;
}
void glew_context::rollback()
{
  current = _rollback;
}

glew_context * glew_context::current = nullptr;
glew_context * glew_context::_rollback = nullptr;


glsl_shader::glsl_shader(const char * * sources, int count, GLenum type)
: id(glCreateShader(type))
{
  ctor_impl(sources, count);
}
glsl_shader::glsl_shader(const char * source, GLenum type)
: id(glCreateShader(type))
{
  ctor_impl(&source, 1);
}
glsl_shader::~glsl_shader()
{
  glDeleteShader(id);
}
glsl_shader::glsl_shader(glsl_shader && other)
: id(other.id)
{
  other.id = 0;
}
glsl_shader & glsl_shader::operator = (glsl_shader && rhs)
{
  GLuint tmp = id;
  id = rhs.id;
  rhs.id = tmp;

  return *this;
}

void glsl_shader::ctor_impl(const char * * sources, int count)
{
  glShaderSource(id, count, sources, nullptr);
  glCompileShader(id);

  GLint status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE)
  {
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length - 1);
    glGetShaderInfoLog(id, length - 1, &length, &log[0]);  // trailing null character is excluded 

    glDeleteShader(id);
    throw std::runtime_error(std::string("glsl_shader compile failure: ")
      + std::string(log.begin(), log.end()));
  }
}



glsl_program::glsl_program(std::initializer_list<glsl_shader> shaders)
: id( glCreateProgram() )
{
  for(auto i = shaders.begin(); i != shaders.end(); ++i)
  {
    glAttachShader(id, i->id);
  }

  glLinkProgram(id);

  GLint status;
  glGetProgramiv(id, GL_LINK_STATUS, &status);
  if(status == GL_FALSE)
  {
    GLint length;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length - 1);
    // trailing null character is excluded 
    glGetProgramInfoLog(id, length - 1, &length, &log[0]);

    glDeleteProgram(id);
    throw std::runtime_error(std::string("glsl_program link failure: ")
      + std::string(log.begin(), log.end()));
  }
}
glsl_program::~glsl_program()
{
  glDeleteProgram(id);
}
void glsl_program::use() const
{
  glUseProgram(id);
}
GLint glsl_program::get_uniform(const char * name) const
{
  GLint uniform = glGetUniformLocation(id, name);
  if(uniform == -1)
    throw std::runtime_error(std::string("Could not bind uniform ")
      + std::string(name));
  return uniform;
}
GLint glsl_program::get_attribute(const char * name) const
{
  GLint attribute = glGetAttribLocation(id, name);
  if(attribute == -1)
    throw std::runtime_error(std::string("Could not bind attribute ")
      + std::string(name));
  return attribute;
}



#include <array>
renderer::renderer(window & w)
: _target(&w),
  ogl( _target->context() ),
  gpu_program{
    glsl_shader(
      {"vertex.glsl"},
      GL_VERTEX_SHADER
    ),
    glsl_shader(
      {"fragment.glsl"},
      GL_FRAGMENT_SHADER
    )
  }
{
  gpu_program.use();

  update_projection( _target->size() );

  _target->event_handler(
    std::bind(&renderer::handle_event, this, std::placeholders::_1)
  );
}
renderer::~renderer()
{
  _target->event_handler(nullptr);
}

window & renderer::target()
{
  return *_target;
}
void renderer::target(window & w)
{
  _target->event_handler(nullptr);
  _target = &w;
  _target->event_handler(
    std::bind(&renderer::handle_event, this, std::placeholders::_1)
  );
  _target->context(ogl);
  update_projection( _target->size() );
}

void renderer::clear()
{
  glClear(GL_COLOR_BUFFER_BIT);
}

const glm::mat3 & renderer::model() const
{
  return _model;
}
void renderer::model(const glm::mat3 & m)
{
  _model = m;
  update_needed = true;
}
const glm::mat3 & renderer::view() const
{
  return _view;
}
void renderer::view(const glm::mat3 & v)
{
  _view = v;
  update_needed = true;
}

#include <glm/gtc/type_ptr.hpp>
void renderer::update()
{
  if(update_needed)
  {
    // Update the viewport
    glm::ivec2 s = _target->size();
    glViewport(0, 0, s.x, s.y);

    // Update mvp matrix
    glm::mat3 mvp = projection*_view*_model;
    glUniformMatrix3fv( gpu_program.get_uniform("mvp"), 1, GL_FALSE,
      glm::value_ptr(mvp) );
    update_needed = false;
  }
}

void renderer::handle_event(const SDL_WindowEvent & event)
{
  if(event.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    update_projection( glm::ivec2(event.data1, event.data2) );
}

#include <glm/gtc/matrix_transform.hpp>
void renderer::update_projection(const glm::ivec2 & new_size)
{
  glm::vec2 half_extents(new_size.x/2.0, new_size.y/2.0);
  glm::mat4 p = glm::ortho(-half_extents.x, half_extents.x,
    -half_extents.y, half_extents.y);
  projection = glm::mat3(
    p[0][0], p[0][1], p[0][3],
    p[1][0], p[1][1], p[1][3],
    p[3][0], p[3][1], p[3][3]
  );
  update_needed = true;
}
