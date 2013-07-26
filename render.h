#ifndef __MALLIE_RENDER_H__
#define __MALLIE_RENDER_H__

#include <vector>
#include <string>

namespace mallie {

struct RenderConfig {
  float fov;
  int   width;
  int   height;
  float eye[3];
  float lookat[3];
  float up[3];

  int   num_passes;
  int   num_photons;  // # of photon to shoot per pass.

  std::string obj_filename;

  RenderConfig() :
    fov(45.0),
    width(512),
    height(512),
    num_passes(10),
    num_photons(10000) {

    eye[0] = 0.0;
    eye[1] = 0.0;
    eye[2] = -5.0;
    lookat[0] = 0.0;
    lookat[1] = 0.0;
    lookat[2] = 0.0;
    up[0] = 0.0;
    up[1] = 1.0;
    up[2] = 0.0;
  }
  
};

void Render(
  const RenderConfig& config,
  std::vector<float>& image);   // out image
 
}

#endif  // __MALLIE_RENDER_H__
