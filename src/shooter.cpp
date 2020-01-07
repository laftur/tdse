#include "shooter.h"


periodic::periodic(float_seconds period__)
: cooldown(0.0f)
{
  period(period__);
}

float_seconds periodic::period() const
{
  return period_;
}
void periodic::period(float_seconds period__)
{
  if(period__.count() <= 0.0f)
    throw std::invalid_argument("period must be greater than zero");
  period_ = period__;
}

void periodic::step(float_seconds time)
{
  // Take from cooldown the time that passed
  cooldown -= time;
}
bool periodic::ready() const
{
  // Negative cooldown means we became ready partway through this step (common).
  return cooldown.count() <= 0.0f;
}
float_seconds periodic::trigger()
{
  float_seconds time_after = -cooldown;
  cooldown += period_;
  return time_after;
}
void periodic::reset()
{
  cooldown = float_seconds(0.0f);
}


shooter::shooter(float_seconds fire_period)
: periodic(fire_period),
  enabled(false)
{}
void shooter::presubstep(bullet_world & world, float_seconds substep_time)
{
  // Step all projectiles
  for(auto i = projectiles.begin(); i != projectiles.end(); )
  {
    // Erase projectiles that collide or expire
    if( i->step(world, substep_time) )
      i = projectiles.erase(i);  // Erase returns next projectile.
    else ++i;
  }

  step(substep_time);
  while( ready() )
  {
    if(enabled)
    {
      float_seconds remainder = trigger();

      // Create a new projectile
      projectiles.emplace_back( fire() );
      // Fire period usually elapses before the end of the substep,
      // so step the new projectile ahead with the remaining substep time
      if( projectiles.back().step(world, remainder) ) projectiles.pop_back();
    }
    else
    {
      reset();
      break;
    }
  }
}
