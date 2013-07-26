#include <cstdio>
#include "hashgrid.h"
#include "render.h"

using namespace mallie;

void
Render(
  const RenderConfig& config,
  std::vector<float>& image)
{
  int height = config.height;
  int width  = config.width;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      image[4*(y*width+x)+0] = 1.0f;
      image[4*(y*width+x)+1] = 1.0f;
      image[4*(y*width+x)+2] = 1.0f;
      image[4*(y*width+x)+3] = 1.0f;
    }
  }

}


