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
#include "biped.h"
#include "turret.h"


class soldier
{
public:
  soldier(projectile_world & physics_, const glm::vec2 & position)
  : actor(position),
    bullet_type(0.1f),
    weapon(bullet_type, 8.0f)
  {
    physics_.add(actor);
  }
  soldier(const soldier &) = delete;
  void operator=(const soldier &) = delete;

  biped actor;
  projectile::properties bullet_type;
  turret weapon;
};

class human_interface
{
public:
  human_interface(window & win)
  : win_(win),
    wasd(0, 0), mouse(0, 0)
  {
    win_.input_handler(
      std::bind(&human_interface::handle_input, this, std::placeholders::_1)
    );
  }
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
  void apply_input(soldier & player)
  {
    // Transform mouse coordinates to world space
    auto window_size = win_.size();
    glm::vec2 view_pos(mouse.x - window_size.x/2.0f,
      window_size.y/2.0f - mouse.y);
    camera default_camera(glm::vec2(0.0f, 0.0f), 0.0f, 10.0f);
    glm::vec2 world_pos = glm::vec3(view_pos, 1.0f)
      *default_camera.transform()
      /default_camera.magnification;
    // Apply control inputs to test_biped
    glm::vec2 player_dist = world_pos - player.actor.position();
    player.weapon.target = glm::atan(player_dist.y, player_dist.x);
    player.actor.force(
      glm::vec2(wasd.x*biped::max_linear_force, wasd.y*biped::max_linear_force)
    );
  }

private:
  glm::ivec2 wasd, mouse;
  window & win_;
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
    soldier player( physics, glm::vec2(0.0f, 0.0f) );

    sdl media_layer(SDL_INIT_VIDEO);
    media_layer.gl_version(1, 4);
    window win( media_layer, "", glm::ivec2(640, 480) );
    shape_renderer ren(win);
    human_interface input(win);
    {
      camera test_camera(player.actor.position(), 0.0f, 10.0f);
      ren.view( test_camera.view() );
    }

    // Construct circle shape
    constexpr int num_circle_vertices = 8;
    std::array<glm::vec2, num_circle_vertices> circle_vertices;
    {
      float angle = 0.0f;
      float increment = ( 2*glm::pi<float>() )/num_circle_vertices;
      for(int i = 0; i < num_circle_vertices; ++i)
      {
        circle_vertices[i].x = biped::size*glm::cos(angle);
        circle_vertices[i].y = biped::size*glm::sin(angle);
        angle += increment;
      }
    }
    std::array<unsigned short, circle_vertices.size()> circle_indices;
    for(unsigned short i = 0; i < circle_indices.size(); ++i)
      circle_indices[i] = i;
    shape test_biped_shape(circle_vertices, circle_indices, GL_LINE_LOOP);

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
      input.apply_input(player);

      // Calculate turret appearance
      float turret_aim = player.weapon.aim_angle();
      glm::vec2 turret_end( biped::size*glm::cos(turret_aim),
        biped::size*glm::sin(turret_aim) );
      segment turret_segment( player.actor.position() );
      turret_segment.end = turret_segment.start + turret_end;

      // Render scene
      ren.clear();
      ren.render(player.actor.model(), test_biped_shape);
      ren.render(turret_segment);
      win.present();

      // Advance
      auto lap_time = timer.lap();
      player.weapon.step(lap_time);
      physics.step(lap_time);
    }
  }
  catch(const std::exception & e)
  {
    std::cout << e.what() << std::endl;
  }
}
