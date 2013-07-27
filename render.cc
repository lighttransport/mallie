#include <cstdio>
#include "hashgrid.h"
#include "render.h"
#include "camera.h"
#include "timerutil.h"
#include "scene.h"

#ifdef _WIN32
#define THREAD_TLS __declspec(thread)
#else // Assume gcc-like compiler
#define THREAD_TLS  __thread
#endif

namespace mallie {

struct PathVertex
{
  real3 P;          // Position
  real3 N;          // Normal
  real3 wi;         // Incident vector
  real3 throughput; // Path throughput(RGB)
  int   matID;      // Material ID
};

typedef std::vector<PathVertex> Path;

namespace {

inline double randomreal(void) {
  // xorshift RNG
  // @fixme { don't use __thread keyword? }
  static unsigned int THREAD_TLS
        x=123456789,y=362436069,z=521288629,w=88675123;
  unsigned t=x^(x<<11);
  x=y; y=z; z=w; w=(w^(w>>19))^(t^(t>>8));
  return w*(1.0/4294967296.0);
}

void GenEyePath(
  const Scene& scene,
  int x,
  int y)
{

  double u0 = randomreal();
  double u1 = randomreal();  

}

void GenLightPath(
  const Scene& scene,
  int numPhotons)
{
  std::vector<Path> paths;

  for (int i = 0; i < numPhotons; i++) {
    Path path;
    paths.push_back(path);
  }
}

}

void Render(
  const Scene& scene,
  const RenderConfig& config,
  std::vector<float>& image,  // RGB
  float eye[3], 
  float lookat[3], 
  float up[3], 
  float quat[4],
  int   step)
{
  int width = config.width;
  int height = config.height;
  float fov = config.fov;

  float origin[3], corner[3], du[3], dv[3];
  Camera camera(eye, lookat, up);
  camera.BuildCameraFrame(origin, corner, du, dv, fov, quat, width, height);
  //printf("[Mallie] origin = %f, %f, %f\n", gOrigin[0], gOrigin[1], gOrigin[2]);
  //printf("[Mallie] corner = %f, %f, %f\n", gCorner[0], gCorner[1], gCorner[2]);
  //printf("[Mallie] du     = %f, %f, %f\n", gDu[0], gDu[1], gDu[2]);
  //printf("[Mallie] dv     = %f, %f, %f\n", gDv[0], gDv[1], gDv[2]);

  assert(image.size() >= 3 * width * height);
  //memset(&image.at(0), 0, sizeof(float) * width * height * 3);

  mallie::timerutil t;
  mallie::timerutil tEventTimer;

  t.start();
  tEventTimer.start();

  //
  // Clear background with gradation.
  //
  memset(&image[0], 0, sizeof(float) * width * height * 3);

  #pragma omp parallel for schedule(dynamic, 1)
  for (int y = 0; y < height; y+=step) {

    if ((y % 100) == 0) {
      printf("\r[Mallie] Render %d of %d", y, height);
      fflush(stdout);
    }

    for (int x = 0; x < width; x++) {

      float u = randomreal();
      float v = randomreal();

      Ray ray;

      real3 dir;
      dir[0] = (corner[0] + (x + u + step/2.0f) * du[0] + ((height - y - 1) - v + step/2.0f) * dv[0]) - eye[0];
      dir[1] = (corner[1] + (x + u + step/2.0f) * du[1] + ((height - y - 1) - v + step/2.0f) * dv[1]) - eye[1];
      dir[2] = (corner[2] + (x + u + step/2.0f) * du[2] + ((height - y - 1) - v + step/2.0f) * dv[2]) - eye[2];
      dir.normalize();
      
      ray.dir = dir;
      ray.org = eye;

      Intersection isect;
      bool hit = scene.Trace(isect, ray);
      
      image[3*(y*width+x)+0] = x / (float)width;
      image[3*(y*width+x)+1] = y / (float)height;
      image[3*(y*width+x)+2] = 0.0;
    }

    // block fill
    if (step > 1) {
      for (int x = 0; x < width; x += step) {
        for (int v = 0; v < step; v++) {
            for (int u = 0; u < step; u++) {
                for (int k = 0; k < 3; k++) {
                    image[((y+v) * width * 3 + (x+u) * 3) + k] = image[3*(y*width+x)+k];
                }
            }
        }
      }
    }

  }

  t.end();

  double fps = 1000.0 / (double)t.msec();
  printf("\r[Mallie] Render time: %f sec(s) | %f fps", (double)t.msec() / 1000.0, fps);
  fflush(stdout);

}

} // namespace
