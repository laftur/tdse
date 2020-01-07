#ifndef SHOOTER_H_INCLUDED
#define SHOOTER_H_INCLUDED
#include "projectile.h"


class periodic
{
public:
  periodic(float_seconds period__);

  float_seconds period() const;
  void period(float_seconds period__);

  // Set how much time passed
  void step(float_seconds time);
  // Returns true if trigger is ready
  bool ready() const;
  // Returns time remaining after triggering
  float_seconds trigger();
  // Forget about remaining triggers this step
  void reset();

  float_seconds cooldown;

private:
  float_seconds period_;
};


#include <list>
class shooter : public periodic, public needs_presubstep
{
public:
  shooter(float_seconds fire_period);

  std::list<projectile> projectiles;
  bool enabled;

protected:
  void presubstep(bullet_world & world, float_seconds substep_time) override;
  virtual projectile fire() = 0;
};


#endif  // SHOOTER_H_INCLUDED
