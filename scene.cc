#include <cassert>

#include "importers/tiny_obj_loader.h"
#include "importers/mesh_loader.h"
#include "scene.h"
#include "timerutil.h"

bool
Scene::Init(
  const std::string& objFilename)
{
  Mesh mesh;
  bool ret = MeshLoader::LoadObj(mesh, objFilename.c_str());
  if (!ret) {
    printf("[Mallie] Failed to load .obj file [ %s ]\n", objFilename.c_str());
    return false;
  }

  float scale = 1.0; // @todo

  for (size_t i = 0; i < mesh.numVertices; i++) {
    mesh.vertices[3*i+0] *= scale;
    mesh.vertices[3*i+1] *= scale;
    mesh.vertices[3*i+2] *= scale;
  }

  mallie::timerutil t;
  t.start();

  BVHBuildOptions options;  // Use default option

  printf("  BVH build option:\n");
  printf("    # of leaf primitives: %d\n", options.minLeafPrimitives);
  printf("    SAH binsize         : %d\n", options.binSize);

  ret = accel_.Build(&mesh, options);
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
