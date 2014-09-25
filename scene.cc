#include <cassert>
#include <limits>
#include <algorithm>

#include "importers/tiny_obj_loader.h"
#include "importers/eson.h"
#include "importers/mesh_loader.h"
#include "scene.h"
#include "timerutil.h"

#ifdef ENABLE_EMBREE
#include "embree2/rtcore_ray.h"
#endif

namespace mallie {

namespace {

#ifdef ENABLE_EMBREE
void error_handler(const RTCError code, const char *str) {
  printf("Embree: ");
  switch (code) {
  case RTC_UNKNOWN_ERROR:
    printf("RTC_UNKNOWN_ERROR");
    break;
  case RTC_INVALID_ARGUMENT:
    printf("RTC_INVALID_ARGUMENT");
    break;
  case RTC_INVALID_OPERATION:
    printf("RTC_INVALID_OPERATION");
    break;
  case RTC_OUT_OF_MEMORY:
    printf("RTC_OUT_OF_MEMORY");
    break;
  case RTC_UNSUPPORTED_CPU:
    printf("RTC_UNSUPPORTED_CPU");
    break;
  default:
    printf("invalid error code");
    break;
  }
  if (str) {
    printf(" (");
    while (*str)
      putchar(*str++);
    printf(")\n");
  }
  abort();
}
#endif
}

void Node::UpdateTransform() {}

Scene::Scene() {}

Scene::~Scene() {
#ifdef ENABLE_EMBREE
  rtcExit();
#endif
  delete[] mesh_.vertices;
  delete[] mesh_.faces;
  delete[] mesh_.materialIDs;
}

bool Scene::Init(const std::string &objFilename,
                 const std::string &esonFilename,
                 const std::string &magicaVoxelFilename,
                 const std::string &materialFilename, double sceneScale) {

  bool ret = false;

  if (!objFilename.empty()) {
    ret = MeshLoader::LoadObj(mesh_, objFilename.c_str());

    if (!ret) {
      printf("Mallie:err\tmsg:Failed to load .obj file [ %s ]\n",
             objFilename.c_str());
      return false;
    } else {
      printf("Mallie:info\tmsg:Success to load .obj file [ %s ]\n",
             objFilename.c_str());
    }
  } else if (!esonFilename.empty()) {
    ret = MeshLoader::LoadESON(mesh_, esonFilename.c_str());
    if (!ret) {
      printf("Mallie:err\tmsg:Failed to load .eson file [ %s ]\n",
             esonFilename.c_str());
      return false;
    } else {
      printf("Mallie:info\tmsg:Success to load .eson file [ %s ]\n",
             esonFilename.c_str());
    }
  } else if (!magicaVoxelFilename.empty()) {
    ret = MeshLoader::LoadMagicaVoxel(mesh_, materials_,
                                      magicaVoxelFilename.c_str());
    if (!ret) {
      printf("Mallie:err\tmsg:Failed to load .vox file [ %s ]\n",
             magicaVoxelFilename.c_str());
      return false;
    } else {
      printf("Mallie:info\tmsg:Success to load .vox file [ %s ]\n",
             magicaVoxelFilename.c_str());
    }
  }

  if (ret == false) {
    printf("Mallie:err\tmsg:Failed to load mesh\n");
    return ret;
  }

  for (size_t i = 0; i < mesh_.numVertices; i++) {
    mesh_.vertices[3 * i + 0] *= sceneScale;
    mesh_.vertices[3 * i + 1] *= sceneScale;
    mesh_.vertices[3 * i + 2] *= sceneScale;
  }

#ifdef ENABLE_EMBREE
  rtcInit(NULL);

  // rtcSetErrorFunction(error_handler);

  scene_ = rtcNewScene(RTC_SCENE_STATIC, RTC_INTERSECT1);
#endif

  mallie::timerutil t;
  t.start();

#ifdef ENABLE_EMBREE

  unsigned int meshID = rtcNewTriangleMesh(scene_, RTC_GEOMETRY_STATIC,
                                           mesh_.numFaces, mesh_.numVertices);

  // Set vertices. Also computes bounding box for BoundingBox().
  bmin_[0] = std::numeric_limits<real>::max();
  bmin_[1] = std::numeric_limits<real>::max();
  bmin_[2] = std::numeric_limits<real>::max();
  bmax_[0] = -std::numeric_limits<real>::max();
  bmax_[1] = -std::numeric_limits<real>::max();
  bmax_[2] = -std::numeric_limits<real>::max();
  float *vertices = (float *)rtcMapBuffer(scene_, meshID, RTC_VERTEX_BUFFER);
  for (size_t i = 0; i < mesh_.numVertices; i++) {
    vertices[4 * i + 0] = mesh_.vertices[3 * i + 0];
    vertices[4 * i + 1] = mesh_.vertices[3 * i + 1];
    vertices[4 * i + 2] = mesh_.vertices[3 * i + 2];
    vertices[4 * i + 3] = 0.0f; // not used.

    bmin_[0] = std::min(bmin_[0], (real)vertices[4 * i + 0]);
    bmin_[1] = std::min(bmin_[1], (real)vertices[4 * i + 1]);
    bmin_[2] = std::min(bmin_[2], (real)vertices[4 * i + 2]);

    bmax_[0] = std::max(bmax_[0], (real)vertices[4 * i + 0]);
    bmax_[1] = std::max(bmax_[1], (real)vertices[4 * i + 1]);
    bmax_[2] = std::max(bmax_[2], (real)vertices[4 * i + 2]);
  }
  rtcUnmapBuffer(scene_, meshID, RTC_VERTEX_BUFFER);

  // Set faces
  int *triangles = (int *)rtcMapBuffer(scene_, meshID, RTC_INDEX_BUFFER);
  for (size_t i = 0; i < mesh_.numFaces; i++) {
    triangles[3 * i + 0] = mesh_.faces[3 * i + 0];
    triangles[3 * i + 1] = mesh_.faces[3 * i + 1];
    triangles[3 * i + 2] = mesh_.faces[3 * i + 2];
  }
  rtcUnmapBuffer(scene_, meshID, RTC_INDEX_BUFFER);

  rtcCommit(scene_);
#else

  BVHBuildOptions options; // Use default option

  printf("  BVH build option:\n");
  printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
  printf("    SAH binsize         : %d\n", options.binSize);

  ret = accel_.Build(&mesh_, options);
  assert(ret);

  BVHBuildStatistics stats = accel_.GetStatistics();

  printf("  BVH statistics:\n");
  printf("    # of leaf   nodes: %d\n", stats.numLeafNodes);
  printf("    # of branch nodes: %d\n", stats.numBranchNodes);
  printf("  Max tree depth   : %d\n", stats.maxTreeDepth);
#endif

  t.end();
  printf("  BVH build time: %d msecs\n", (int)t.msec());

  real3 bmin, bmax;
  BoundingBox(bmin, bmax);
  printf("  BVH bounding box:\n");
  printf("    bmin = (%f, %f, %f)\n", bmin[0], bmin[1], bmin[2]);
  printf("    bmax = (%f, %f, %f)\n", bmax[0], bmax[1], bmax[2]);

  return true;
}

bool Scene::Trace(Intersection &isect, Ray &ray) {
#ifdef ENABLE_EMBREE

  // Convert to Embree's ray structure.
  RTCRay r;
  r.org[0] = ray.org[0];
  r.org[1] = ray.org[1];
  r.org[2] = ray.org[2];
  r.dir[0] = ray.dir[0];
  r.dir[1] = ray.dir[1];
  r.dir[2] = ray.dir[2];
  r.tnear = 0.0f;
  r.tfar = std::numeric_limits<real>::infinity();
  r.geomID = RTC_INVALID_GEOMETRY_ID;
  r.primID = RTC_INVALID_GEOMETRY_ID;
  r.mask = -1;
  r.time = 0;

  rtcIntersect(scene_, r);

  bool hit = (r.geomID != RTC_INVALID_GEOMETRY_ID);

  if (hit) {
    isect.position[0] = r.org[0] + r.tfar * r.dir[0];
    isect.position[1] = r.org[1] + r.tfar * r.dir[1];
    isect.position[2] = r.org[2] + r.tfar * r.dir[2];

    isect.t = r.tfar;

    isect.faceID = r.primID;
    isect.materialID = mesh_.materialIDs[r.primID];

    isect.f0 = mesh_.faces[3 * isect.faceID + 0];
    isect.f1 = mesh_.faces[3 * isect.faceID + 1];
    isect.f2 = mesh_.faces[3 * isect.faceID + 2];

    // calc geometric normal.
    real3 p0, p1, p2;
    p0[0] = mesh_.vertices[3 * isect.f0 + 0];
    p0[1] = mesh_.vertices[3 * isect.f0 + 1];
    p0[2] = mesh_.vertices[3 * isect.f0 + 2];
    p1[0] = mesh_.vertices[3 * isect.f1 + 0];
    p1[1] = mesh_.vertices[3 * isect.f1 + 1];
    p1[2] = mesh_.vertices[3 * isect.f1 + 2];
    p2[0] = mesh_.vertices[3 * isect.f2 + 0];
    p2[1] = mesh_.vertices[3 * isect.f2 + 1];
    p2[2] = mesh_.vertices[3 * isect.f2 + 2];

    real3 p10 = p1 - p0;
    real3 p20 = p2 - p0;
    real3 n = vcross(p10, p20);
    n.normalize();

    isect.geometricNormal = n;

    isect.normal = n;
  }

  return hit;
#else
  return accel_.Traverse(isect, &mesh_, ray);
#endif
}

void Scene::BoundingBox(real3 &bmin, real3 &bmax) {
#ifdef ENABLE_EMBREE
  bmin = bmin_;
  bmax = bmax_;
#else
  const std::vector<BVHNode> &nodes = accel_.GetNodes();
  assert(nodes.size() > 0);

  bmin[0] = nodes[0].bmin[0];
  bmin[1] = nodes[0].bmin[1];
  bmin[2] = nodes[0].bmin[2];

  bmax[0] = nodes[0].bmax[0];
  bmax[1] = nodes[0].bmax[1];
  bmax[2] = nodes[0].bmax[2];
#endif
}

real3 Scene::GetBackgroundRadiance(real3 &dir) {
  // Constant dome light
  return real3(0.75, 0.75, 0.75);
}

} // namespace
