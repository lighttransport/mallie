#ifndef __INTERSECTION_H__
#define __INTERSECTION_H__

#include "common.h"

typedef struct {
  real t;
  real u;
  real v;
  unsigned int faceID;

  unsigned int f0;
  unsigned int f1;
  unsigned int f2;

  real3 position;
  real3 geometricNormal;
  real3 normal;
  real3 tangent;
  real3 binormal;
  real texcoord[2];

} Intersection;

#endif // __INTERSECTION_H__
