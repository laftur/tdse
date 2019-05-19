#ifndef SHOOTER_H_INCLUDED
#define SHOOTER_H_INCLUDED
#include "projectile.h"


class periodic : public needs_presubstep
{
public:
  periodic(float_seconds period__);
  periodic(const periodic & other);
  periodic & operator=(const periodic & rhs);

  float_seconds period() const;
  void period(float_seconds period__);

  bool enabled;
  const float_seconds & cooldown;
  
protected:
  void presubstep(bullet_world & world, float_seconds substep_time) override;
  virtual void trigger(bullet_world & world, float_seconds remainder) = 0;

private:
  float_seconds cooldown_;
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
