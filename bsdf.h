#ifndef __MALLIE_BSDF_H__
#define __MALLIE_BSDF_H__

#include "common.h"

class BSDF {
  typedef enum {
    kDiffuse = 1,
    kGlossy = 2,
    kReflect = 4,
    kRefract = 8,
  } Type;

  real3 Eval();
  real3 Pdf();
  real3 Sample();

  bool IsDelta() const { return isDelta; }

  real3 N;
  real3 Wo;
  real3 Wi;
  Type type;

  int materialID;

  bool isDelta;
};

#endif // __MALLIE_BSDF_H__
