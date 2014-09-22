#ifndef __MALLIE_PRIM_PLANE_HPP__
#define __MALLIE_PRIM_PLANE_HPP__

#include "intersection.h"
#include "common.h"

namespace mallie {

class Plane {
public:
  Plane() {}

  ~Plane() {}

  void set(float a, float b, float c, float d) {
    m_a = a;
    m_b = b;
    m_c = c;
    m_d = d;
  }

  bool intersect(Intersection *info, const Ray &ray);

  float m_a, m_b, m_c, m_d;
};
}

#endif // __MALLIE_PLANE_HPP__
