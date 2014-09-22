#include <cassert>

#include "importers/tiny_obj_loader.h"
#include "importers/eson.h"
#include "importers/mesh_loader.h"
#include "scene.h"
#include "timerutil.h"

namespace mallie {

void Node::UpdateTransform() {}

Scene::~Scene() {
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
    ret = MeshLoader::LoadMagicaVoxel(mesh_, magicaVoxelFilename.c_str());
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

  mallie::timerutil t;
  t.start();

  BVHBuildOptions options; // Use default option

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

bool Scene::Trace(Intersection &isect, Ray &ray) {
  return accel_.Traverse(isect, &mesh_, ray);
}

void Scene::BoundingBox(real3 &bmin, real3 &bmax) {
  const std::vector<BVHNode> &nodes = accel_.GetNodes();
  assert(nodes.size() > 0);

  bmin[0] = nodes[0].bmin[0];
  bmin[1] = nodes[0].bmin[1];
  bmin[2] = nodes[0].bmin[2];

  bmax[0] = nodes[0].bmax[0];
  bmax[1] = nodes[0].bmax[1];
  bmax[2] = nodes[0].bmax[2];
}

real3 Scene::GetBackgroundRadiance(real3 &dir) {
  // Constant dome light
  return real3(0.75, 0.75, 0.75);
}

} // namespace
