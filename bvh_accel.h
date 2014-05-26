#ifndef __BVH_ACCEL_H__
#define __BVH_ACCEL_H__

#include <vector>

#include "common.h"
#include "mesh.h"
#include "intersection.h"

class BVHNode {
public:
  BVHNode() {};
  ~BVHNode() {};

  real bmin[3];
  real bmax[3];

  int flag; // 1 = leaf node, 0 = branch node
  int axis;

  // leaf
  //   data[0] = npoints
  //   data[1] = index
  //
  // branch
  //   data[0] = child[0]
  //   data[1] = child[1]
  unsigned int data[2];
};

///< BVH build option.
struct BVHBuildOptions {
  bool debugPrint;
  real costTaabb;
  int minLeafPrimitives;
  int maxTreeDepth;
  int binSize;

  // Set default value: Taabb = 0.2
  BVHBuildOptions()
      : costTaabb(0.2), minLeafPrimitives(16), maxTreeDepth(256), binSize(64) {}
};

///< BVH build statistics.
struct BVHBuildStatistics {
  int maxTreeDepth;
  int numLeafNodes;
  int numBranchNodes;

  // Set default value: Taabb = 0.2
  BVHBuildStatistics() : maxTreeDepth(0), numLeafNodes(0), numBranchNodes(0) {}
};

class BVHAccel {
public:
  BVHAccel() {};
  ~BVHAccel() {};

  ///< Build BVH for input mesh.
  bool Build(const Mesh *mesh, const BVHBuildOptions &options);

  ///< Get statistics of built BVH tree. Valid after Build()
  BVHBuildStatistics GetStatistics() const { return stats_; }

  ///< Dump built BVH to the file.
  bool Dump(const char *filename);

  /// Load BVH binary
  bool Load(const char *filename);

  ///< Traverse into BVH along ray and find closest hit point if found
  bool Traverse(Intersection &isect, const Mesh *mesh, Ray &ray);

  const std::vector<BVHNode> &GetNodes() const { return nodes_; }
  const std::vector<unsigned int> &GetIndices() const { return indices_; }

private:
  ///< Builds BVH tree recursively.
  size_t BuildTree(const Mesh *mesh, unsigned int leftIdx,
                   unsigned int rightIdx, int depth);

  BVHBuildOptions options_;
  std::vector<BVHNode> nodes_;
  std::vector<unsigned int> indices_; // max 4G triangles.
  BVHBuildStatistics stats_;
};

#endif // __BVH_ACCEL_H__
       // vim:set sw=2 ts=2 expandtab:
