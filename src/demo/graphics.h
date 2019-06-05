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
#ifndef GRAPHICS_H_INCLUDED
#define GRAPHICS_H_INCLUDED


/* GLEW_MX requires that glewGetContext return a pointer to the current
 * GLEWcontext.
 * Class glew_context records that state, and is used by glewGetContext.
 * Class glew_context is only useful for GLEW_MX, but MX support was dropped in
 * version 2.0. The code remains in case MX support is reintroduced in a later
 * version.
 * */

#include <GL/glew.h>

// GLEW multi-context support
#ifdef GLEW_MX
GLEWContext * glewGetContext();
#endif

class glew_context {
public:
  glew_context();
  ~glew_context();
  glew_context(const glew_context &) = delete;
  void operator = (const glew_context &) = delete;

  void make_current();

  static void set_rollback();
  static void rollback();

private:
  static glew_context * current;
  static glew_context * _rollback;

// GLEW multi-context support
#ifdef GLEW_MX
  GLEWContext context;

  friend GLEWContext * glewGetContext();
#endif
};


#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>
class glsl_program;
class glsl_shader {
public:
  // Constructor accepting character arrays containing shader sources
  glsl_shader(const char * * sources, int count, GLenum type);
  // Constructor accepting one array containing one shader source
  glsl_shader(const char * source, GLenum type);
  ~glsl_shader();
  glsl_shader(glsl_shader && other);
  glsl_shader & operator = (glsl_shader && rhs);
  glsl_shader(const glsl_shader &) = delete;
  void operator = (const glsl_shader &) = delete;

private:
  GLuint id;

  void ctor_impl(const char * * sources, int count);

  friend class glsl_program;

public:
  // Templated constructor accepting a list of file names containing sources
  template <class T>
  glsl_shader(const T & fileNames, GLenum type)
  : id(glCreateShader(type))
  {
    std::vector<std::vector<char>> sources;

    for(auto fileName = fileNames.begin();
      fileName != fileNames.end();
      ++fileName)
    {
      std::ifstream sourceFile(*fileName);
      if(!sourceFile.is_open()) {
        glDeleteShader(id);
        throw std::runtime_error(
          "glsl_shader compile failure: File open failure"
        );
      }

      sourceFile.seekg(0, sourceFile.end);
      auto length = sourceFile.tellg();
      sourceFile.seekg(0, sourceFile.beg);

      sources.emplace_back(length);
      sourceFile.read(&sources.back()[0], length);
      sources.back().push_back('\0');
    }

    const char * safeptr = &sources[0][0];
    ctor_impl(&safeptr, sources.size());
  }
  // For some reason, the compiler has a hard time deducing that the above
  // templated function can accept an initializer_list.
  class init_list_wrapper : public std::initializer_list<const char *> {
  public:
    init_list_wrapper(const std::initializer_list<const char *> & parent)
    : initializer_list(parent) {}
    init_list_wrapper(std::initializer_list<const char *> && parent)
    : initializer_list(std::move(parent)) {}
  };
  glsl_shader(std::initializer_list<const char *> file_names, GLenum type)
  : glsl_shader(init_list_wrapper(file_names), type)
  {
    // All work delegated
  }
};


class glsl_program {
public:
  glsl_program(std::initializer_list<glsl_shader> shaders);
  ~glsl_program();
  glsl_program(const glsl_program &) = delete;
  void operator = (const glsl_program &) = delete;

  void use() const;
  GLint get_uniform(const char * name) const;
  GLint get_attribute(const char * name) const;

private:
  GLuint id;
};


#include "window.h"
#include "glm.h"
class renderer {
public:
  // Constructor and destructor
  renderer(window & w);
  ~renderer();

  // Renderers are not copyable or movable
  renderer(const renderer &) = delete;
  renderer & operator=(const renderer &) = delete;

  // Get/set target window for rendering
  // Setting causes this rendering context to become current
  window & target();
  void target(window & w);

  // Clear the drawing area
  void clear();

  // Get/set model/view matrix
  const glm::mat3 & model() const;
  void model(const glm::mat3 & m);
  const glm::mat3 & view() const;
  void view(const glm::mat3 & v);

  // Compose mvp matrix and send it to the gpu
  void update();
  // This rendering context must be current before calls to update
  // This context is current after construction and after setting target

private:
  window * _target;
  ogl_context ogl;
  glew_context glew;

public:
  glsl_program gpu_program;

private:
  glm::mat3 _model, _view, projection;
  bool update_needed;

  // Call to notify of window events such as minimization or a size change
  void handle_event(const SDL_WindowEvent & event);

  // Set the viewport to the target window's dimensions
  void update_viewport();
  // Set the projection matrix to match the target window's dimensions
  void update_projection(const glm::ivec2 & new_size);
};


#endif  // GRAPHICS_H_INCLUDED
