#ifndef __MALLIE_LIGHT_H__
#define __MALLIE_LIGHT_H__

#include "common.h"

#include <cassert>

namespace mallie {

class Light {

  virtual real3 Radiance() const = 0;

  // Finite(area) or not(directional, envmap)
  virtual bool IsFinite() const = 0;

  // Whether the light has delta function(point, directional) or not(area)
  virtual bool IsDelta() const = 0;

  // Sample light position and direction, and its PDFs.
  virtual real SampleL(real posRnd[2], real dirRnd[2]) const = 0;
};

class AreaLight : public Light {
public:
  AreaLight(const real3 &corner, const real3 &du, const real3 &dv) {
    assert(0); // @todo
  }

  virtual real3 Radiance() const {
    assert(0);
    return real3(0.0, 0.0, 0.0);
  }

  bool IsFinite() const { return false; }

  bool IsDelta() const { return false; }

  real SampleL(real posRnd[2], real dirRnd[2]) const;
};

} // namespace

#endif // __MALLIE_LIGHT_H__
