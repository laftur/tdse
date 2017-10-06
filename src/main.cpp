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
#include <stdexcept>
#include <iostream>
#include "camera.h"
#include "shape_renderer.h"
#include "ship.h"
#include "turret.h"


class player
{
public:
  player(window & win, projectile_world & physics_)
  : win_(win),
    wasd(0, 0), mouse(0, 0),
    test_ship( glm::vec2(0.0f, 0.0f) )
  {
    physics_.add(test_ship);
    win_.input_handler(
      std::bind(&player::handle_input, this, std::placeholders::_1)
    );
  }
  player(const player &) = delete;
  void operator=(const player &) = delete;

  void handle_input(const SDL_Event & event)
  {
    switch(event.type)
    {
    case SDL_KEYDOWN:
      if(event.key.repeat) break;
      switch(event.key.keysym.scancode)
      {
      case SDL_SCANCODE_W:
        wasd.y += 1;
        break;
      case SDL_SCANCODE_A:
        wasd.x -= 1;
        break;
      case SDL_SCANCODE_S:
        wasd.y -= 1;
        break;
      case SDL_SCANCODE_D:
        wasd.x += 1;
        break;
      }
      break;
    case SDL_KEYUP:
      if(event.key.repeat) break;
      switch(event.key.keysym.scancode)
      {
      case SDL_SCANCODE_W:
        wasd.y -= 1;
        break;
      case SDL_SCANCODE_A:
        wasd.x += 1;
        break;
      case SDL_SCANCODE_S:
        wasd.y += 1;
        break;
      case SDL_SCANCODE_D:
        wasd.x -= 1;
        break;
      }
      break;
    case SDL_MOUSEMOTION:
      mouse.x = event.motion.x;
      mouse.y = event.motion.y;
      break;
    }
  }
  void apply_input()
  {
    // Apply control inputs to test_ship
    test_ship.force( glm::vec2(wasd.y*ship::max_linear_force, 0.0f) );
    test_ship.target_angle =
      angle_from_mat2( test_ship.real_orientation() ) - wasd.x;
  }

private:
  window & win_;
  glm::ivec2 wasd, mouse;

public:
  ship test_ship;
};


#include <chrono>
#include <thread>
class lap_timer
{
public:
  typedef std::chrono::steady_clock::time_point time_point;
  typedef std::chrono::steady_clock::duration duration;

  lap_timer();
  time_point last() const;
  duration lap();

private:
  time_point _last;
};

lap_timer::lap_timer()
: _last( std::chrono::steady_clock::now() )
{}
lap_timer::time_point lap_timer::last() const
{
  return _last;
}
lap_timer::duration lap_timer::lap()
{
  time_point __last = _last;
  _last = std::chrono::steady_clock::now();
  return _last - __last;
}


#include <glm/gtc/constants.hpp>
int main(int argc, char * argv[])
{
  try
  {
    bullet_components physics_parts;
    projectile_world physics(physics_parts);

    sdl media_layer(SDL_INIT_VIDEO);
    media_layer.gl_version(1, 4);
    window win( media_layer, "", glm::ivec2(640, 480) );
    shape_renderer ren(win);
    player input(win, physics);
    {
      camera test_camera(input.test_ship.position(), 0.0f, 10.0f);
      ren.view( test_camera.view() );
    }

    std::array<unsigned short, ship::triangle_vertices.size()> triangle_indices;
    for(unsigned short i = 0; i < triangle_indices.size(); ++i)
      triangle_indices[i] = i;
    shape ship_shape(ship::triangle_vertices, triangle_indices, GL_LINE_LOOP);

    bool quit = false;
    lap_timer timer;
    while(!quit)
    {
      SDL_Event event;
      while( media_layer.poll(event) )
      {
        switch(event.type)
        {
        case SDL_QUIT:
          quit = true;
          break;
        }
      }
      input.apply_input();

      // Render scene
      ren.clear();
      ren.render(input.test_ship.model(), ship_shape);
      win.present();

      // Advance
      auto lap_time = timer.lap();
      physics.step(lap_time);
    }
  }
  catch(const std::exception & e)
  {
    std::cout << e.what() << std::endl;
  }
}
