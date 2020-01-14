#include "player.h"


player::player()
: movement(0.0f, 0.0f),
  aim(0.0f),
  fire(false)
{}
void player::apply_input(ship & subject)
{
  subject.force( glm::vec2(movement.y*ship::max_linear_force, 0.0f) );
  if(movement.x == 0.0f)
  {
    subject.rctrl.stop = true;
    subject.rctrl_active = true;
  }
  else
  {
    subject.torque(-movement.x*ship::max_torque);
    subject.rctrl_active = false;
  }
}
void player::apply_input(warship & subject)
{
  apply_input( static_cast<ship &>(subject) );
  subject.weapon_tree.fire(fire);
}
void player::apply_input(biped & subject)
{
  subject.force( glm::vec2(
    movement.x*biped::max_linear_force,
    movement.y*biped::max_linear_force
  ) );
}
void player::apply_input(soldier & subject)
{
  apply_input( static_cast<biped &>(subject) );
  subject.weapon.target = glm::atan(aim.y, aim.x);
  subject.enabled = fire;
}


local_player::local_player(sdl & media_layer)
: interface( media_layer, "TDSE demo", glm::ivec2(640, 480) ),
  view(glm::vec2(0.0f, 0.0f), 0.0f, 40.0f)
{
  interface.input_handler(
    std::bind(&local_player::handle_input, this, std::placeholders::_1)
  );
}

void local_player::handle_input(const SDL_Event & event)
{
  switch(event.type)
  {
  case SDL_KEYDOWN:
    if(event.key.repeat) break;
    switch(event.key.keysym.scancode)
    {
    case SDL_SCANCODE_W:
      movement.y += 1.0f;
      break;
    case SDL_SCANCODE_A:
      movement.x -= 1.0f;
      break;
    case SDL_SCANCODE_S:
      movement.y -= 1.0f;
      break;
    case SDL_SCANCODE_D:
      movement.x += 1.0f;
      break;
    case SDL_SCANCODE_SPACE:
      fire = true;
      break;

    case SDL_SCANCODE_R:
      view.magnification_velocity += 1.0f;
      break;
    case SDL_SCANCODE_F:
      view.magnification_velocity -= 1.0f;
      break;
    }
    break;
  case SDL_KEYUP:
    if(event.key.repeat) break;
    switch(event.key.keysym.scancode)
    {
    case SDL_SCANCODE_W:
      movement.y -= 1.0f;
      break;
    case SDL_SCANCODE_A:
      movement.x += 1.0f;
      break;
    case SDL_SCANCODE_S:
      movement.y += 1.0f;
      break;
    case SDL_SCANCODE_D:
      movement.x -= 1.0f;
      break;
    case SDL_SCANCODE_SPACE:
      fire = false;
      break;

    case SDL_SCANCODE_R:
      view.magnification_velocity -= 1.0f;
      break;
    case SDL_SCANCODE_F:
      view.magnification_velocity += 1.0f;
      break;
    }
    break;
  case SDL_MOUSEMOTION:
    // Transform mouse coordinates to aim angle
    {
      auto window_size = interface.size();
      aim = view.orientation*glm::vec2(
        event.motion.x - window_size.x/2.0f,
        window_size.y/2.0f - event.motion.y
      );
    }
    break;
  }
}
