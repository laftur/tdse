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
#include "window.h"
#include <stdexcept>
#include <iostream>


ogl_context::~ogl_context()
{
  SDL_GL_DeleteContext(_context);
}

ogl_context::ogl_context(ogl_context && other)
: _context(other._context)
{
  other._context = nullptr;
}
ogl_context & ogl_context::operator=(ogl_context && rhs)
{
  _context = rhs._context;
  rhs._context = nullptr;
}

ogl_context::ogl_context(SDL_GLContext context)
: _context(context)
{}


window::window(sdl & media_layer, const char * title, const glm::ivec2 & _size,
  bool fullscreen)
: _window(
    SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      _size.x, _size.y,
      (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) |
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    )
  ),
  media_layer_(media_layer)
{
  if(_window == nullptr) throw std::runtime_error(std::string("SDL_CreateWindow: ") + std::string(SDL_GetError()));
  media_layer_.register_window(*this);
}
window::~window()
{
  media_layer_.unregister_window(*this);
  SDL_DestroyWindow(_window);
}

void window::present() const
{
  SDL_GL_SwapWindow(_window);
}

ogl_context window::context()
{
  SDL_GLContext new_context = SDL_GL_CreateContext(_window);
  if(new_context == nullptr)
    throw std::runtime_error(std::string("SDL_GL_CreateContext: ")
      + std::string(SDL_GetError()));

  return ogl_context(new_context);
}
void window::context(const ogl_context & context)
{
  if( SDL_GL_MakeCurrent(_window, context._context) )
    throw std::runtime_error(std::string("SDL_GL_MakeCurrent: ")
      + std::string(SDL_GetError()));
}

glm::ivec2 window::size() const
{
  int x, y;
  SDL_GetWindowSize(_window, &x, &y);
  return glm::ivec2(x, y);
}
void window::size(const glm::ivec2 & _size)
{
  SDL_SetWindowSize(_window, _size.x, _size.y);
}

void window::event_handler(const std::function<void(const SDL_WindowEvent &)>
  & handler) const
{
  event_handler_ = handler;
}
const std::function<void(const SDL_WindowEvent &)>
  & window::event_handler() const
{
  return event_handler_;
}
void window::input_handler(const std::function<void(const SDL_Event &)>
  & handler) const
{
  input_handler_ = handler;
}
const std::function<void(const SDL_Event &)> & window::input_handler() const
{
  return input_handler_;
}


sdl::sdl(Uint32 flags)
{
  SDL_Init(flags);
}
sdl::~sdl()
{
  SDL_Quit();
}

bool sdl::poll(SDL_Event & event)
{
  SDL_Event event_;
  while(SDL_PollEvent(&event_))
  {
    switch(event_.type)
    {
    case SDL_WINDOWEVENT:
      {
        window & w = *windows.at(event_.window.windowID);
        if(event_.window.event == SDL_WINDOWEVENT_CLOSE)
        {
          if(w.input_handler_) w.input_handler_(event_);
        }
        else if(w.event_handler_) w.event_handler_(event_.window);
      }
      break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      {
        window & w = *windows.at(event_.key.windowID);
        if(w.input_handler_) w.input_handler_(event_);
      }
      break;
    case SDL_MOUSEMOTION:
      {
        window & w = *windows.at(event_.motion.windowID);
        if(w.input_handler_) w.input_handler_(event_);
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      {
        window & w = *windows.at(event_.button.windowID);
        if(w.input_handler_) w.input_handler_(event_);
      }
      break;
    case SDL_MOUSEWHEEL:
      {
        window & w = *windows.at(event_.wheel.windowID);
        if(w.input_handler_) w.input_handler_(event_);
      }
      break;

    default:
      event = event_;
      return true;
    }
  }
  return false;
}

void sdl::gl_version(int major, int minor, bool core)
{
  if(core) SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
}

void sdl::register_window(window & w)
{
  windows.insert(
    std::make_pair(
      SDL_GetWindowID(w._window),
      &w
    )
  );
}
void sdl::unregister_window(window & w)
{
  auto found = windows.find(SDL_GetWindowID(w._window));
  if(found == windows.end())
    throw std::runtime_error("sdl::unregister_window: \
      Attempted to unregister a window not in registry");
  windows.erase(found);
}
