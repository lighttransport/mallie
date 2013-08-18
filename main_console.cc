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
  Scene& scene,
  const RenderConfig& config)
{
  printf("[Mallie] Console mode\n");
  std::vector<float> image;

  int width = config.width;
  int height = config.height;

  image.resize(width*height*3);

  //Render(config, image, width, height, config.eye, config.lookat, config.up);

  std::string outfilename("output.jpg"); // fixme

  std::vector<unsigned char> out;
  HDRToLDR(out, image, width, height);
  //SaveAsJPEG(outfilename.c_str(), out, width, height);

  printf("[Mallie] Output %s\n", outfilename.c_str());

}

} // liina
