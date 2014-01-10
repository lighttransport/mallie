#include <cstdio>
#include "hashgrid.h"
#include "render.h"
#include "camera.h"
#include "timerutil.h"
#include "scene.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef _WIN32
#define THREAD_TLS __declspec(thread)
#else // Assume gcc-like compiler
#define THREAD_TLS  __thread
#endif

namespace mallie {

const double kFar = 1.0e+30;
const double kEPS = 1.0e-3;
const int    kMaxPathLength = 16;
const int    kMinPathLength = 2;

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

unsigned int gSeed[1024][4];

inline void init_randomreal(void) {
#if _OPENMP
  assert(omp_get_max_threads() < 1024);

  for (int i = 0; i < omp_get_max_threads(); i++) {
    gSeed[i][0] = 123456789 + i;
    gSeed[i][1] = 362436069;
    gSeed[i][2] = 521288629;
    gSeed[i][3] = 88675123;
  }
#else
#endif
}

inline double randomreal(void) {
  // xorshift RNG
#ifdef _OPENMP
  int tid = omp_get_thread_num();
  unsigned int x = gSeed[tid][0];
  unsigned int y = gSeed[tid][1];
  unsigned int z = gSeed[tid][2];
  unsigned int w = gSeed[tid][3];
  unsigned t=x^(x<<11);
  x=y; y=z; z=w; w=(w^(w>>19))^(t^(t>>8));

  gSeed[tid][0] = x;
  gSeed[tid][1] = y;
  gSeed[tid][2] = z;
  gSeed[tid][3] = w;
  return w*(1.0/4294967296.0);
#else
  // @fixme { don't use __thread keyword? }
  static unsigned int THREAD_TLS
        x=123456789,y=362436069,z=521288629,w=88675123;
  unsigned t=x^(x<<11);
  x=y; y=z; z=w; w=(w^(w>>19))^(t^(t>>8));
  return w*(1.0/4294967296.0);
#endif
}

static void
GenerateBasis(
  real3&        tangent,
  real3&        binormal,
  const real3&  normal)
{
  // Find the minor axis of the vector
  int i;
  int index = -1;
  double minval = 1.0e+6;
  double val = 0;

  for (int i = 0; i < 3; i++) {
    val = fabsf(normal[i]);
    if (val < minval) {
      minval = val;
      index  = i;
    }
  }

  if (index == 0) {

    tangent.x = 0.0;
    tangent.y = -normal.z;
    tangent.z = normal.y;
    tangent.normalize();

    binormal = vcross(tangent, normal);
    binormal.normalize();

  } else if (index == 1) {

    tangent.x = -normal.z;
    tangent.y = 0.0;
    tangent.z = normal.x;
    tangent.normalize();

    binormal = vcross(tangent, normal);
    binormal.normalize();

  } else {

    tangent.x = -normal.y;
    tangent.y = normal.x;
    tangent.z = 0.0;
    tangent.normalize();

    binormal = vcross(tangent, normal);
    binormal.normalize();
  }

}

// Importance sample diffuse BRDF.
double
SampleDiffuseIS(
  real3& dir,
  const real3& normal)
{
  real3 tangent, binormal;

  GenerateBasis(tangent, binormal, normal);

  double theta = acos(sqrt(1.0 - randomreal()));
  double phi   = 2.0 * M_PI * randomreal();

  double cosTheta = cos(theta);

  /* D = T*cos(phi)*sin(theta) + B*sin(phi)*sin(theta) + N*cos(theta) */
  double cos_theta = cos(theta);
  real3 T  = tangent * cos(phi) * sin(theta);
  real3 B  = binormal * sin(phi) * sin(theta);
  real3 N  = normal * (cos_theta);

  dir = T + B + N;

  return cos_theta; // PDF = weight
}

// Mis power (1 for balance heuristic)
double Mis(double aPdf)
{
    return aPdf;
}

// Mis weight for 2 pdfs
double Mis2(
    double aSamplePdf,
    double aOtherPdf)
{
    return Mis(aSamplePdf) / (Mis(aSamplePdf) + Mis(aOtherPdf));
}

void GenEyePath(
  const Scene& scene,
  int x,
  int y)
{

  double u0 = randomreal();
  double u1 = randomreal();  

}

void TraceRay(
  const Scene& scene,
  Ray& ray)
{

}

void GenLightPath(
  Scene& scene,
  int numPhotons)
{
  std::vector<Path> paths;

  real3 lightPos = real3(0.0, 20.0, 0.0);
  real3 lightDir = real3(0.0, -1.0, 0.0);

  for (int i = 0; i < numPhotons; i++) {
    Path path;

    Ray ray;

    real3 dir = lightDir;
    dir.normalize();
      
    ray.dir = dir;
    ray.org = lightPos;

    Intersection isect;
    bool hit = scene.Trace(isect, ray);

    paths.push_back(path);
  }
}


real3
PathTrace(
  Scene& scene,
  const Camera& camera,
  const RenderConfig& config,
  std::vector<float>& image,  // RGB
  int px, int py,
  int step)
{
  //
  // 1. Sample eye(E0)
  //
  float u = randomreal() - 0.5;
  float v = randomreal() - 0.5;

  Ray ray = camera.GenerateRay(px+u+step/2.0f, py+v+step/2.0f);

  Intersection isect;
  isect.t = kFar; 

  real3         throughput;
  real3         radiance = real3(0.0, 0.0, 0.0);
  unsigned int  pathLength = 1;
  bool          lastSpecular = true;
  double        lastPdfW = 1.0;

  for (;; ++pathLength) {
    bool hit = scene.Trace(isect, ray);
    if (!hit) {

      if (pathLength < kMinPathLength) {
        // eye -> background hit.
        break;
      }
    
      // Hit background.
      real3 kd = real3(0.5, 0.5, 0.5);
      radiance += kd / real3(pathLength, pathLength, pathLength);
    }

    if (pathLength >= kMaxPathLength) {
      break;
    }

    real3 hitP = ray.org + isect.t * ray.dir;

    // 2. Next event estimation
    {

    }

    // 3. Continue path tracing.
    {
      double r = randomreal();
      real3 sampledDir;
      
      // faceforward.
      real3 n = isect.normal;
      double ndoti = vdot(isect.normal, ray.dir.neg());
      if (ndoti < 0.0) {
        n = n.neg();
      }
      double pdf = SampleDiffuseIS(sampledDir, n);

      //throughput *= factor * (cosThetaOut / pdf);

      ray.org = hitP + kEPS * sampledDir;
      ray.dir = sampledDir;

      isect.t = kFar;
    }

  }

  return radiance;
}

}

void Render(
  Scene& scene,
  const RenderConfig& config,
  std::vector<float>& image,  // RGB
  const double eye[3], 
  const double lookat[3], 
  const double up[3], 
  const double quat[4],
  int   step)
{
  int width = config.width;
  int height = config.height;
  double fov = config.fov;

  double origin[3], corner[3], du[3], dv[3];
  Camera camera(eye, lookat, up);
  camera.BuildCameraFrame(origin, corner, du, dv, fov, quat, width, height);
  //printf("[Mallie] origin = %f, %f, %f\n", gOrigin[0], gOrigin[1], gOrigin[2]);
  //printf("[Mallie] corner = %f, %f, %f\n", gCorner[0], gCorner[1], gCorner[2]);
  //printf("[Mallie] du     = %f, %f, %f\n", gDu[0], gDu[1], gDu[2]);
  //printf("[Mallie] dv     = %f, %f, %f\n", gDv[0], gDv[1], gDv[2]);

  assert(image.size() >= 3 * width * height);
  //memset(&image.at(0), 0, sizeof(float) * width * height * 3);
  

  init_randomreal();

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

#if 1
      real3 radiance = PathTrace(scene, camera, config, image, x, y, step);

      image[3*(y*width+x)+0] = radiance[0];
      image[3*(y*width+x)+1] = radiance[1];
      image[3*(y*width+x)+2] = radiance[2];

#else
      float u = randomreal() - 0.5;
      float v = randomreal() - 0.5;

      Ray ray = camera.GenerateRay(x+u+step/2.0f, y+v+step/2.0f);

      Intersection isect;
      bool hit = scene.Trace(isect, ray);

      if (hit) {

        double dotNI = fabs(vdot(isect.normal, ray.dir.neg()));

        image[3*(y*width+x)+0] = isect.normal[0];
        image[3*(y*width+x)+1] = isect.normal[1];
        image[3*(y*width+x)+2] = isect.normal[2];
      }
#endif

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
