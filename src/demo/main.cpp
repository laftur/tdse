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


#include <array>
#include "physics.h"
class obstacle : public body
{
public:
  static const std::array<glm::vec2, 4> square_vertices;
  static const btConvexHullShape square_prism;
  static const btConvex2dShape square;

  obstacle(const glm::vec2 & position);
};
const std::array<glm::vec2, 4> obstacle::square_vertices =
{
  glm::vec2(1.0f, 1.0f),
  glm::vec2(-1.0f, 1.0f),
  glm::vec2(-1.0f, -1.0f),
  glm::vec2(1.0f, -1.0f)
};
const btConvexHullShape obstacle::square_prism =
  make_convex_hull(square_vertices);
const btConvex2dShape obstacle::square(
  const_cast<btConvexHullShape *>(&square_prism)
);
obstacle::obstacle(const glm::vec2 & position)
: body( 0.0f, square, compose_transform(position) )
{}
#include <vector>
class obstacle_grid
{
public:
  obstacle_grid( const glm::vec2 & origin,
                 const glm::ivec2 & size,
                 const glm::vec2 & spacing = glm::vec2(10.0f, 10.0f) );
  void add_all(bullet_world & physics);
  void remove_all(bullet_world & physics);

  const std::vector<obstacle> & obstacles;

private:
  std::vector<obstacle> obstacles_;
};
obstacle_grid::obstacle_grid(const glm::vec2 & origin,
                             const glm::ivec2 & size,
                             const glm::vec2 & spacing)
: obstacles(obstacles_)
{
  obstacles_.reserve(size.x*size.y);
  for(int x = 0; x < size.x; ++x)
    for(int y = 0; y < size.y; ++y)
      obstacles_.emplace_back( origin + glm::vec2(x*spacing.x, y*spacing.y) );
}
void obstacle_grid::add_all(bullet_world & physics)
{
  for(auto i = obstacles_.begin(); i != obstacles_.end(); ++i)
    physics.add_body(*i);
}
void obstacle_grid::remove_all(bullet_world & physics)
{
  for(auto i = obstacles_.begin(); i != obstacles_.end(); ++i)
    physics.remove_body(*i);
}


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
#include "player.h"
#include "shape_renderer.h"
void soldier_demo()
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
    soldier player_body(glm::vec2(0.0f, 0.0f),
                        projectile::properties(0.008f, 1000.0f),
                        prand);

    // Move player body based on collision dynamics
    physics.add_body(player_body);
    // Apply movement controls in between substeps
    physics.add_callback( static_cast<biped &>(player_body) );
    // Create and manage projectiles in between substeps
    physics.add_callback( static_cast<shooter &>(player_body) );

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
      // Move target bodies based on collision dynamics
      physics.add_body( test_bipeds.back() );
    }

    sdl media_layer(SDL_INIT_VIDEO);
    media_layer.gl_version(1, 4);
    local_player player_io(media_layer);
    shape_renderer ren(player_io.interface);

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
      // Calculate turret appearance
      float turret_aim = player_body.weapon.aim_angle;
      glm::vec2 turret_end( biped::size*glm::cos(turret_aim),
        biped::size*glm::sin(turret_aim) );
      segment turret_segment( player_body.position() );
      turret_segment.end = turret_segment.start + turret_end;

      // Calculate segments from projectiles
      std::vector<segment> psegments;
      for(auto i = player_body.projectiles.begin();
          i != player_body.projectiles.end();
          ++i)
        psegments.emplace_back(
          i->position(),
          i->position() - 0.01f*i->velocity()
        );

      // Set camera to follow player object
      player_io.view.position = player_body.position();
      // Synchronize view with the camera
      ren.view( player_io.view.view() );
      // Clear screen
      ren.clear();
      // Draw the player
      ren.render(player_body.model(), test_biped_shape);
      // Draw the target bipeds
      for(auto i = test_bipeds.begin(); i != test_bipeds.end(); ++i)
        ren.render(i->model(), test_biped_shape);
      // Draw the player's weapon direction
      ren.render(turret_segment);
      // Draw projectiles in-flight
      ren.render(psegments);
      // Flip all drawings to the screen
      player_io.interface.present();

      // Time how long the last frame required
      auto lap_time = timer.lap();
      // Update player's weapon direction
      player_body.weapon.step(lap_time);
      // Move physics objects (including projectiles), fire new projectiles,
      // apply forces, react to being shot
      physics.step(lap_time);
      // Update camera position/orientation
      player_io.view.step(lap_time);

      // Process user input
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
      player_io.apply_input(player_body);
    }
  }
  catch(const std::exception & e)
  {
    std::cout << e.what() << std::endl;
  }
}
void ship_demo()
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
    ship opponent( compose_transform(glm::vec2(60.0f, 60.0f)) );

    warship player_body( compose_transform(glm::vec2(0.0f, 0.0f)), prand );
    const projectile::properties test_bullet(0.008f, 1000.0f);
    player_body.weapon_tree.weapons.emplace_back(
      glm::vec2(0.0f,  0.25f), test_bullet
    );
    player_body.weapon_tree.weapons.emplace_back(
      glm::vec2(0.0f, -0.25f), test_bullet
    );

    // Move ships based on collision dynamics
    physics.add_body(player_body);
    physics.add_body(opponent);
    // Apply movement controls and fire weapons in between substeps
    physics.add_callback(player_body);

    // obstacles
    std::array<glm::vec2, 4> square_vertices = {
      glm::vec2(1.0f, 1.0f),
      glm::vec2(-1.0f, 1.0f),
      glm::vec2(-1.0f, -1.0f),
      glm::vec2(1.0f, -1.0f)
    };
    auto square_prism = make_convex_hull(square_vertices);
    btConvex2dShape square(&square_prism);
    obstacle_grid squares( glm::vec2(-55.0f, -55.0f),
      glm::ivec2(12, 12) );
    squares.add_all(physics);

    sdl media_layer(SDL_INIT_VIDEO);
    media_layer.gl_version(1, 4);
    local_player player_io(media_layer);
    shape_renderer ren(player_io.interface);

    // rendering shapes
    std::array<unsigned short, square_vertices.size()> square_indices;
    for(unsigned short i = 0; i < square_indices.size(); ++i)
      square_indices[i] = i;
    shape square_shape(square_vertices, square_indices, GL_LINE_LOOP);
    std::array<unsigned short, ship::triangle_vertices.size()> triangle_indices;
    for(unsigned short i = 0; i < triangle_indices.size(); ++i)
      triangle_indices[i] = i;
    shape ship_shape(ship::triangle_vertices, triangle_indices, GL_LINE_LOOP);

    bool quit = false;
    lap_timer timer;
    while(!quit)
    {
      // Calculate segments from projectiles
      std::vector<segment> psegments;
      for(auto i = player_body.projectiles.begin();
          i != player_body.projectiles.end();
          ++i)
        psegments.emplace_back(
          i->position(),
          i->position() - 0.01f*i->velocity()
        );

      // Set camera to follow player object
      player_io.view.position = player_body.position();
      // Synchronize view with the camera
      ren.view( player_io.view.view() );
      // Clear screen
      ren.clear();
      // Draw the player
      ren.render(player_body.model(), ship_shape);
      // Draw opponent
      ren.render(opponent.model(), ship_shape);
      // Draw obstacles
      std::vector<glm::mat3> models;
      models.reserve( squares.obstacles.size() );
      for(auto i = squares.obstacles.begin();
          i < squares.obstacles.end();
          ++i)
        models.push_back( i->model() );
      ren.render(models, square_shape);
      // Draw projectiles in-flight
      ren.render(psegments);
      // Flip all drawings to the screen
      player_io.interface.present();

      // Time how long the last frame required
      auto lap_time = timer.lap();
      // Move physics objects (including projectiles), fire new projectiles,
      // apply forces, react to being shot
      physics.step(lap_time);
      // Update camera position/orientation
      player_io.view.step(lap_time);

      // Process user input
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
      player_io.apply_input(player_body);
    }
  }
  catch(const std::exception & e)
  {
    std::cout << e.what() << std::endl;
  }
}
#include "config.h"
#include "string.h"
int main(int argc, char * argv[])
{
  const char * usage_message =
    "Usage:\n"
    "projectile demo:  demo\n"
    "spaceship demo:   demo space";

  switch(argc)
  {
  case 2:
    if( ! strcmp(argv[1], "space") ) ship_demo();
    else std::cout << usage_message << std::endl;
    break;
  default:
    soldier_demo();
  }
}
