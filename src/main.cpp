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


class human_interface
{
public:
  human_interface(window & win)
  : win_(win),
    wasd(0, 0), mouse(0, 0), zoom(0), space(false),
    cam(glm::vec2(0.0f, 0.0f), 0.0f, 40.0f)
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
      case SDL_SCANCODE_SPACE:
        space = true;
        break;
      case SDL_SCANCODE_R:
        zoom += 1;
        break;
      case SDL_SCANCODE_F:
        zoom -= 1;
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
      case SDL_SCANCODE_SPACE:
        space = false;
        break;
      case SDL_SCANCODE_R:
        zoom -= 1;
        break;
      case SDL_SCANCODE_F:
        zoom += 1;
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
    glm::vec2 world_pos = cam.transform()
      *glm::vec3(view_pos/cam.magnification, 1.0f);
    // Apply control inputs to player object
    glm::vec2 player_dist = world_pos - player.position();
    player.weapon.target = glm::atan(player_dist.y, player_dist.x);
    player.force(
      glm::vec2(wasd.x*biped::max_linear_force, wasd.y*biped::max_linear_force)
    );
    player.wants_fire = space;
    
    cam.magnification_velocity = zoom*1.0f;
  }

private:
  window & win_;
  glm::ivec2 wasd, mouse;
  float zoom;
  bool space;

public:
  kinematic_camera cam;
};


#include <chrono>
#include <thread>
class lap_timer
{
public:
  typedef std::chrono::steady_clock::time_point time_point;
  typedef std::chrono::steady_clock::duration duration;

  lap_timer()
  : _last( std::chrono::steady_clock::now() )
  {}
  time_point last() const
  {
    return _last;
  }
  duration lap()
  {
    time_point __last = _last;
    _last = std::chrono::steady_clock::now();
    return _last - __last;
  }

private:
  time_point _last;
};


#include <glm/gtc/constants.hpp>
int main(int argc, char * argv[])
{
  try
  {
    std::random_device::result_type seed;
    {
      std::random_device r;
      seed = r();
    }
    std::default_random_engine prand(seed);
    bullet_world physics;
    soldier player(glm::vec2(0.0f, 0.0f), prand);

    // Register collision dynamics
    physics.add( static_cast<body &>(player) );
    // Movement controls are applied in between substeps
    physics.add( static_cast<needs_presubstep &>(static_cast<biped &>(player)) );
    // Projectiles are created and managed in between substeps
    physics.add( static_cast<shooter &>(player) );

    // Instantiate targets to shoot at
    std::vector<biped> test_bipeds;
    static const glm::vec2 start(-6.25f, -6.25f);
    static const int width = 5;
    static const int num_test_bipeds = 25;
    static const float spacing = 2.5f;
    test_bipeds.reserve(num_test_bipeds);
    for(int i = 0; i < num_test_bipeds; ++i)
    {
      test_bipeds.emplace_back(
        start + glm::vec2( spacing*(i%width), spacing*(i/width) )
      );
      // Register collision dynamics
      physics.add( static_cast<body &>(test_bipeds.back()) );
      // No need to register for movement controls (targets are dummies)
      // physics.add( static_cast<biped &>(test_bipeds.back()) );
    }

    sdl media_layer(SDL_INIT_VIDEO);
    media_layer.gl_version(1, 4);
    window win( media_layer, "", glm::ivec2(640, 480) );
    shape_renderer ren(win);
    human_interface input(win);

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
      segment turret_segment( player.position() );
      turret_segment.end = turret_segment.start + turret_end;

      // Calculate segments from projectiles
      std::vector<segment> psegments;
      for(auto i = player.projectiles.begin();
          i != player.projectiles.end();
          ++i)
        psegments.emplace_back(i->position, i->position - 0.01f*i->velocity);

      // Clear screen
      ren.clear();
      // Synchronize view with the camera
      ren.view( input.cam.view() );
      // Draw the player
      ren.render(player.model(), test_biped_shape);
      // Draw the target bipeds
      for(auto i = test_bipeds.begin(); i != test_bipeds.end(); ++i)
        ren.render(i->model(), test_biped_shape);
      // Draw the player's weapon direction
      ren.render(turret_segment);
      // Draw projectiles in-flight
      ren.render(psegments);
      // Flip all drawings to the screen
      win.present();

      // Time how long the last frame required
      auto lap_time = timer.lap();
      // Change the player's weapon direction
      player.weapon.step(lap_time);
      // Move physics objects (including projectiles), fire new projectiles,
      // apply forces, react to being shot
      physics.step(lap_time);
      // Update camera position/orientation
      input.cam.step(lap_time);
    }
  }
  catch(const std::exception & e)
  {
    std::cout << e.what() << std::endl;
  }
}
