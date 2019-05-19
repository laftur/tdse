#include "shooter.h"


periodic::periodic(float_seconds period__)
: enabled(false),
  cooldown(cooldown_),
  cooldown_(0.0f),
  period_(period__)
{}
periodic::periodic(const periodic & other)
: enabled(false),
  cooldown(cooldown_),
  cooldown_(other.cooldown_),
  period_(other.period_)
{}
periodic & periodic::operator=(const periodic & rhs)
{
  cooldown_ = rhs.cooldown_;
  period_ = rhs.period_;

  return *this;
}

float_seconds periodic::period() const
{
  return period_;
}
void periodic::period(float_seconds period__)
{
  if(period__ > bullet_world::fixed_substep)
    period_ = period__;
  else
    period_ = bullet_world::fixed_substep;
}

void periodic::presubstep(bullet_world & world, float_seconds substep_time)
{
  cooldown_ -= substep_time;
  if(cooldown_.count() <= 0.0f)
  {
    // Period elapsed
    if(enabled)
    {
      trigger(world, -cooldown_);

      // Cool down until period has elapsed
      cooldown_ += period_;
    }
    else cooldown_ = float_seconds(0.0f);
  }
}


shooter::shooter(float_seconds fire_period)
: periodic(fire_period)
{}
void shooter::presubstep(bullet_world & world, float_seconds substep_time)
{
  // Projectiles outside boundary (square half-extents) are deleted
  static constexpr float boundary = 1000.0f;

  // Step all projectiles
  for(auto i = projectiles.begin(); i != projectiles.end(); )
  {
    // Erase projectiles that pass boundary or collide
    auto position = i->position;
    if( position.x >  boundary ||
        position.x < -boundary ||
        position.y >  boundary ||
        position.y < -boundary ||
        i->step(world, substep_time) )
      i = projectiles.erase(i);  // Erase returns next projectile.
    else ++i;
  }

  periodic::presubstep(world, substep_time);
}

void shooter::trigger(bullet_world & world, float_seconds remainder)
{
  // Create a new projectile
  projectiles.emplace_back( fire() );
  // Fire period usually elapses before the end of the substep,
  // so step the new projectile ahead with the remaining substep time
  if( projectiles.back().step(world, remainder) ) projectiles.pop_back();
}
