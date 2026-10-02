#ifndef SUN_H
#define SUN_H

#include "body.h"


class Sun : public Body
{
  public:
    Sun(const std::string& name, const double& mass, const double& radius)
    : Body(name, "0",
           false, Tu::PER_DAY,
           mass, mass, mass,
           radius,
           0., 0.,
           0., 0.,
           0., 0.,
           0., 0.,
           0., 0.,
           0., 0.)
    {}

    virtual void time_evol() override
    {
        return ;
    }
};

#endif // SUN_H
