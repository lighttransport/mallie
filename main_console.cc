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
#include <sstream>

#include "camera.h"
#include "timerutil.h"
#include "render.h"
#include "jpge.h"

namespace mallie {

namespace {

inline unsigned char fclamp(float x) {
  int i = x * 255.5;
  if (i < 0)
    return 0;
  if (i > 255)
    return 255;
  return (unsigned char)i;
}

void HDRToLDR(std::vector<unsigned char> &out, const std::vector<float> &in,
              const std::vector<int> &in_count, int width, int height) {
  out.resize(width * height * 3);
  assert(in.size() == (width * height * 3));

  // Simple [0, 1] -> [0, 255]
  for (int i = 0; i < width * height * 3; i++) {
    out[i] = fclamp(in[i] / in_count[i / 3]);
  }
}

void SaveAsJPEG(const char *filename,
                std::vector<unsigned char> &image, // RGB
                int width, int height) {
  jpge::params comp_params;
  comp_params.m_quality = 100;
  bool ret = jpge::compress_image_to_jpeg_file(filename, width, height, 3,
                                               &image.at(0), comp_params);
  assert(ret);
}

} // local

void DoMainConsole(Scene &scene, const RenderConfig &config) {
  printf("[Mallie] Console mode\n");
#if 0
  {
    std::vector<float> image;
    std::vector<int> count;

    const int width = config.width;
    const int height = config.height;

    image.resize(width * height * 3);
    count.resize(width * height);

    mallie::Render(scene, config, image, count, config.eye, config.lookat, config.up, config.quat, 1);

    std::string outfilename("output.jpg"); // fixme

    std::vector<unsigned char> out;
    HDRToLDR(out, image, count, width, height);
    SaveAsJPEG(outfilename.c_str(), out, width, height);

    printf("[Mallie] Output %s\n", outfilename.c_str());
  }
#else
  const_cast<RenderConfig &>(config).width = 600;
  const_cast<RenderConfig &>(config).height = 600;

  const_cast<RenderConfig &>(config).eye[0] = 0;
  const_cast<RenderConfig &>(config).eye[1] = 1;
  const_cast<RenderConfig &>(config).eye[2] = -4;

  const int total_frame = 1;
  for (int i = 0; i < total_frame; ++i) {
    std::vector<float> image;
    std::vector<int> count;

    int width = config.width;
    int height = config.height;

    image.resize(width * height * 3);
    count.resize(width * height);

    double eye[3];

    /*eye[0] = config.eye[0];
    eye[1] = config.eye[1];
    eye[2] = config.eye[2];*/

    eye[0] = 4.0 * sin(2.0 * M_PI * i / total_frame);
    eye[1] = 1.0;
    eye[2] = 4.0 * cos(2.0 * M_PI * i / total_frame);

    // mallie::RenderPanoramic(scene, config, image, count, eye, config.lookat,
    // config.up, config.quat, /* stereo = */ false);
    mallie::RenderPanoramic(scene, config, image, count, eye, config.lookat,
                            config.up, config.quat, /* stereo = */ true);

    std::string outfilename;
    {
      std::stringstream ss;
      ss << "output" << i << ".jpg";
      outfilename = ss.str();
    }

    std::vector<unsigned char> out;
    HDRToLDR(out, image, count, width, height);
    SaveAsJPEG(outfilename.c_str(), out, width, height);

    printf("[Mallie] Output %s\n", outfilename.c_str());
  }
#endif
}

} // liina
