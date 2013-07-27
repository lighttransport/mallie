#include <cassert>

#include "importers/tiny_obj_loader.h"
#include "importers/mesh_loader.h"
#include "scene.h"
#include "timerutil.h"

namespace mallie {

bool
Scene::Init(
  const std::string& objFilename,
  const std::string& materialFilename,
  double sceneScale)
{
  bool ret = MeshLoader::LoadObj(mesh_, objFilename.c_str());
  if (!ret) {
    printf("[Mallie] Failed to load .obj file [ %s ]\n", objFilename.c_str());
    return false;
  }

  for (size_t i = 0; i < mesh_.numVertices; i++) {
    mesh_.vertices[3*i+0] *= sceneScale;
    mesh_.vertices[3*i+1] *= sceneScale;
    mesh_.vertices[3*i+2] *= sceneScale;
  }

  mallie::timerutil t;
  t.start();

  BVHBuildOptions options;  // Use default option

  printf("  BVH build option:\n");
  printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
  printf("    SAH binsize         : %d\n", options.binSize);

  ret = accel_.Build(&mesh_, options);
  assert(ret);

  t.end();
  printf("  BVH build time: %d msecs\n", (int)t.msec());

  BVHBuildStatistics stats = accel_.GetStatistics();

  printf("  BVH statistics:\n");
  printf("    # of leaf   nodes: %d\n", stats.numLeafNodes);
  printf("    # of branch nodes: %d\n", stats.numBranchNodes);
  printf("  Max tree depth   : %d\n", stats.maxTreeDepth);

  return true;
}

const bool
Scene::Trace(
  Intersection& isect,
  const Ray& ray) const
{
  return false;
}

} // namespace
