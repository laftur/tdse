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
#ifndef __WINDOW_H
#define __WINDOW_H


#include <SDL2/SDL.h>
#include "glm.h"


class window;
class ogl_context
{
public:
  // Destructor
  ~ogl_context();

  // OpenGL contexts are not copyable
  ogl_context(const ogl_context & other) = delete;
  ogl_context & operator=(const ogl_context & rhs) = delete;

  // Move constructor/assignment operator
  ogl_context(ogl_context && other);
  ogl_context & operator=(ogl_context && rhs);

private:
  // Instances are created by class window
  friend class window;
  ogl_context(SDL_GLContext context);

  SDL_GLContext _context;
};


#include <functional>
class sdl;
class window
{
public:
  // Constructor and destructor
  window(sdl & media_layer, const char * title, const glm::ivec2 & _size,
    bool fullscreen = false);
  ~window();

  // Windows are not copyable or movable
  window(const window & other) = delete;
  window & operator=(const window & rhs) = delete;

  // Flip the drawing surface to the display
  void present() const;

  // Create a new opengl context associated with this window
  // Makes the new context current
  ogl_context context();
  // Associate an existing opengl context with this window
  // Makes said context current
  void context(const ogl_context & context);

  // Get/set the size of this window
  glm::ivec2 size() const;
  void size(const glm::ivec2 & _size);

  // Get/set function to handle this window's events
  void event_handler(const std::function<void(const SDL_WindowEvent &)>
    & handler) const;
  const std::function<void(const SDL_WindowEvent &)> & event_handler() const;
  // Get/set function to handle this window's input events
  void input_handler(const std::function<void(const SDL_Event &)>
    & handler) const;
  const std::function<void(const SDL_Event &)> & input_handler() const;

private:
  SDL_Window * _window;
  sdl & media_layer_;

  // Invoke _event_handler for each window event
  mutable std::function<void(const SDL_WindowEvent &)> event_handler_;
  // Invoke input_handler_ for each input event
  mutable std::function<void(const SDL_Event &)> input_handler_;
  friend class sdl;
};


#include <unordered_map>
class sdl
{
public:
  // Constructor and destructor
  sdl(Uint32 flags);
  ~sdl();

  // SDL is not copyable or movable
  sdl(const sdl &) = delete;
  void operator = (const sdl &) = delete;

  bool poll(SDL_Event & event);

  void gl_version(int major, int minor, bool core = true);

private:
  friend class window;
  void register_window(window & w);
  void unregister_window(window & w);
  std::unordered_map<Uint32, window *> windows;
};


#endif
