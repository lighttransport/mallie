#ifndef __MALLIE_LIGHT_H__
#define __MALLIE_LIGHT_H__

#include "common.h"

class
Light
{
  
  virtual real3 Radiance() const = 0;

  // Finite(area) or not(directional, envmap)
  virtual real3 IsFinite() const = 0;

  // Whether the light has delta function(point, directional) or not(area)
  virtual real3 IsDelta() const = 0;

};

class
AreaLight : public Light
{
  public:
    AreaLight(const real3& corner, const real3& du, const real3& dv) {
      assert(0); // @todo
    }
};

#endif // __MALLIE_LIGHT_H__
