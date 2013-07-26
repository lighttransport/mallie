//
// Copyright 2013 Light Transport Entertainment Inc.
//

#ifdef _OPENMP
#include <omp.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <vector>

#include "camera.h"
#include "timerutil.h"
#include "render.h"

namespace mallie {

namespace {

void Render(
  const RenderConfig& config,
  std::vector<float>& image,
  int width,
  int height,
  const float eye[3],
  const float lookat[3],
  const float up[3])
{
  int step = 1;

  float origin[3];
  float corner[3];
  float du[3];
  float dv[3];
  float currQuat[4] = {0.0, 0.0, 0.0, 1.0};
  float fov = config.fov;

  printf("eye    = %f, %f, %f\n", eye[0], eye[1], eye[2]);
  printf("lookat = %f, %f, %f\n", lookat[0], lookat[1], lookat[2]);
  printf("up     = %f, %f, %f\n", up[0], up[1], up[2]);
  printf("fov    = %f\n", fov);

  Camera camera(eye, lookat, up);
  camera.BuildCameraFrame(origin, corner, du, dv, fov, currQuat, width, height);

  printf("origin = %f, %f, %f\n", origin[0], origin[1], origin[2]);
  printf("corner = %f, %f, %f\n", corner[0], corner[1], corner[2]);
  printf("du     = %f, %f, %f\n", du[0], du[1], du[2]);
  printf("dv     = %f, %f, %f\n", dv[0], dv[1], dv[2]);

  mallie::timerutil t;

  t.start();

  #pragma omp parallel for schedule(dynamic, 1)
  for (int y = 0; y < height; y++) {

    if ((y % 100) == 0) {
      printf("[Mallie] Render %d of %d\n", y, height);
      fflush(stdout);
    }
 
    assert(0); // todo
  }

  t.end();

  printf("[Mallie] Render time: %f sec(s)\n", (double)t.msec() / 1000.0);

}

inline unsigned char fclamp(float x)
{
  int i = x * 255.5;
  if (i < 0) return 0;
  if (i > 255) return 255;
  return (unsigned char)i;
}

void
HDRToLDR(
  std::vector<unsigned char>& out,
  const std::vector<float>& in,
  int width,
  int height)
{
  out.resize(width*height*3);
  assert(in.size() == (width*height*3));

  // Simple [0, 1] -> [0, 255]
  for (int i = 0; i < width * height * 3; i++) {
    out[i] = fclamp(in[i]);
  }
}

} // local

void
DoMainConsole(
  const RenderConfig& config)
{
  printf("[Mallie] Console mode\n");
  std::vector<float> image;

  int width = config.width;
  int height = config.height;

  image.resize(width*height*3);

  Render(config, image, width, height, config.eye, config.lookat, config.up);

  std::string outfilename("output.jpg"); // fixme

  std::vector<unsigned char> out;
  HDRToLDR(out, image, width, height);
  //SaveAsJPEG(outfilename.c_str(), out, width, height);

  printf("[Mallie] Output %s\n", outfilename.c_str());

}

} // liina
