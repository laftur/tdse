#ifndef SHOOTER_H_INCLUDED
#define SHOOTER_H_INCLUDED
#include "projectile.h"


class periodic : public needs_presubstep
{
public:
  periodic(float_seconds period__);

  float_seconds period() const;
  void period(float_seconds period__);

  bool enabled;
  float_seconds cooldown;
  
protected:
  void presubstep(bullet_world & world, float_seconds substep_time) override;
  virtual void trigger(bullet_world & world, float_seconds remainder) = 0;

private:
  float_seconds period_;
};


#include <list>
class shooter : public periodic
{
public:
  shooter(float_seconds fire_period);

  std::list<projectile> projectiles;

protected:
  void presubstep(bullet_world & world, float_seconds substep_time) override;
  virtual projectile fire() = 0;
  void trigger(bullet_world & world, float_seconds remainder) override;
};


#endif  // SHOOTER_H_INCLUDED
