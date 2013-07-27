#include <cassert>

#include "importers/tiny_obj_loader.h"
#include "importers/mesh_loader.h"
#include "scene.h"
#include "timerutil.h"

namespace mallie {

Scene::~Scene()
{
  delete [] mesh_.vertices;
  delete [] mesh_.faces;
  delete [] mesh_.materialIDs;
}

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

bool
Scene::Trace(
  Intersection& isect,
  Ray& ray)
{
  return accel_.Traverse(isect, &mesh_, ray);
}

void
Scene::BoundingBox(
  real3& bmin,
  real3& bmax)
{
  const std::vector<BVHNode>& nodes = accel_.GetNodes();
  assert(nodes.size() > 0);

  bmin[0] = nodes[0].bmin[0];
  bmin[1] = nodes[0].bmin[1];
  bmin[2] = nodes[0].bmin[2];

  bmax[0] = nodes[0].bmax[0];
  bmax[1] = nodes[0].bmax[1];
  bmax[2] = nodes[0].bmax[2];
}

} // namespace
