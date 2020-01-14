#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "biped.h"
#include "ship.h"
class player
{
public:
  player();
  void apply_input(ship & subject);
  void apply_input(warship & subject);
  void apply_input(biped & subject);
  void apply_input(soldier & subject);

  glm::vec2 movement, aim;
  bool fire;
};


#include <random>
#include "window.h"
#include "camera.h"
class local_player : public player
{
public:
  local_player(sdl & media_layer);

  window interface;
  kinematic_camera view;

private:
  void handle_input(const SDL_Event & event);
};


#endif  // PLAYER_H_INCLUDED
