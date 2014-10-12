#ifndef __MALLIE_MATERIAL_H__
#define __MALLIE_MATERIAL_H__

#include "common.h"

struct Material {
  real3 diffuse;
  real3 reflection;
  real3 refraction;
  int id;

  Material() {
	  diffuse[0] = 0.5;
	  diffuse[1] = 0.5;
	  diffuse[2] = 0.5;
	  reflection[0] = 0.0;
	  reflection[1] = 0.0;
	  reflection[2] = 0.0;
	  refraction[0] = 0.0;
	  refraction[1] = 0.0;
	  refraction[2] = 0.0;
	  id = -1;
  }
};

#endif // __MALLIE_MATERIAL_H__
