#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <limits>
#include <functional>
#include <algorithm>

#include "bvh_accel.h"

#define ENABLE_TRACE_PRINT (0)
#define ENABLE_DEBUG_PRINT (0)

#define trace(f, ...)                                                          \
  {                                                                            \
    if (ENABLE_TRACE_PRINT)                                                    \
      printf(f, __VA_ARGS__);                                                  \
  }

#if ENABLE_DEBUG_PRINT
#define debug(f, ...)                                                          \
  { printf(f, __VA_ARGS__); }
#else
#define debug(f, ...)
#endif

//
// SAH functions
//

struct BinBuffer {

  BinBuffer(int size) {
    binSize = size;
    bin.resize(2 * 3 * size);
    clear();
  }

  void clear() { memset(&bin[0], 0, sizeof(size_t) * 2 * 3 * binSize); }

  std::vector<size_t> bin; // (min, max) * xyz * binsize
  int binSize;

};

static inline double CalculateSurfaceArea(const real3 &min, const real3 &max) {
  real3 box = max - min;
  return 2.0 * (box[0] * box[1] + box[1] * box[2] + box[2] * box[0]);
}

static inline void GetBoundingBoxOfTriangle(real3 &bmin, real3 &bmax,
                                            const Mesh *mesh,
                                            unsigned int index) {
  unsigned int f0 = mesh->faces[3 * index + 0];
  unsigned int f1 = mesh->faces[3 * index + 1];
  unsigned int f2 = mesh->faces[3 * index + 2];

  real3 p[3];

  p[0] = real3(&mesh->vertices[3 * f0]);
  p[1] = real3(&mesh->vertices[3 * f1]);
  p[2] = real3(&mesh->vertices[3 * f2]);

  bmin = p[0];
  bmax = p[0];

  for (int i = 1; i < 3; i++) {
    bmin[0] = std::min(bmin[0], p[i][0]);
    bmin[1] = std::min(bmin[1], p[i][1]);
    bmin[2] = std::min(bmin[2], p[i][2]);

    bmax[0] = std::max(bmax[0], p[i][0]);
    bmax[1] = std::max(bmax[1], p[i][1]);
    bmax[2] = std::max(bmax[2], p[i][2]);
  }
}

static void ContributeBinBuffer(BinBuffer *bins, // [out]
                                const real3 &sceneMin, const real3 &sceneMax,
                                const Mesh *mesh, unsigned int *indices,
                                unsigned int leftIdx, unsigned int rightIdx) {
  static const real EPS = std::numeric_limits<real>::epsilon() * 1024;

  real binSize = (real) bins->binSize;

  // Calculate extent
  real3 sceneSize, sceneInvSize;
  sceneSize = sceneMax - sceneMin;
  for (int i = 0; i < 3; ++i) {
    assert(sceneSize[i] >= 0.0);

    if (sceneSize[i] > EPS) {
      sceneInvSize[i] = binSize / sceneSize[i];
    } else {
      sceneInvSize[i] = 0.0;
    }
  }

  // Clear bin data
  memset(&bins->bin[0], 0, sizeof(2 * 3 * bins->binSize));

  size_t idxBMin[3];
  size_t idxBMax[3];

  for (size_t i = leftIdx; i < rightIdx; i++) {

    //
    // Quantize the position into [0, BIN_SIZE)
    //
    // q[i] = (int)(p[i] - scene_bmin) / scene_size
    //
    real3 bmin;
    real3 bmax;

    GetBoundingBoxOfTriangle(bmin, bmax, mesh, indices[i]);

    real3 quantizedBMin = (bmin - sceneMin) * sceneInvSize;
    real3 quantizedBMax = (bmax - sceneMin) * sceneInvSize;

    // idx is now in [0, BIN_SIZE)
    for (size_t j = 0; j < 3; ++j) {
      idxBMin[j] = (unsigned int) floor(quantizedBMin[j]);
      idxBMax[j] = (unsigned int) floor(quantizedBMax[j]);

      if (idxBMin[j] >= binSize)
        idxBMin[j] = binSize - 1;
      if (idxBMax[j] >= binSize)
        idxBMax[j] = binSize - 1;

      assert(idxBMin[j] < binSize);
      assert(idxBMax[j] < binSize);

      // Increment bin counter
      bins->bin[0 * (bins->binSize * 3) + j * bins->binSize + idxBMin[j]] += 1;
      bins->bin[1 * (bins->binSize * 3) + j * bins->binSize + idxBMax[j]] += 1;
    }
  }
}

static inline double SAH(size_t ns1, real leftArea, size_t ns2, real rightArea,
                         real invS, real Taabb, real Ttri) {
  //const real Taabb = 0.2f;
  //const real Ttri = 0.8f;
  real T;

  T = 2.0f * Taabb + (leftArea * invS) * (real)(ns1) * Ttri +
      (rightArea * invS) * (real)(ns2) * Ttri;

  return T;
}

static bool FindCutFromBinBuffer(real *cutPos,     // [out] xyz
                                 int &minCostAxis, // [out]
                                 const BinBuffer *bins, const real3 &bmin,
                                 const real3 &bmax, size_t numTriangles,
                                 real costTaabb)   // should be in [0.0, 1.0]
    {
  const real eps = std::numeric_limits<real>::epsilon() * 1024;

  size_t left, right;
  real3 bsize, bstep;
  real3 bminLeft, bmaxLeft;
  real3 bminRight, bmaxRight;
  real saLeft, saRight, saTotal;
  real pos;
  real minCost[3];

  real costTtri = 1.0 - costTaabb;

  minCostAxis = 0;

  bsize = bmax - bmin;
  bstep = bsize * (1.0 / bins->binSize);
  saTotal = CalculateSurfaceArea(bmin, bmax);

  real invSaTotal = 0.0;
  if (saTotal > eps) {
    invSaTotal = 1.0 / saTotal;
  }

  for (int j = 0; j < 3; ++j) {

    //
    // Compute SAH cost for right side of each cell of the bbox.
    // Exclude both extreme side of the bbox.
    //
    //  i:      0    1    2    3
    //     +----+----+----+----+----+
    //     |    |    |    |    |    |
    //     +----+----+----+----+----+
    //

    real minCostPos = bmin[j] + 0.5 * bstep[j];
    minCost[j] = std::numeric_limits<real>::max();

    left = 0;
    right = numTriangles;
    bminLeft = bminRight = bmin;
    bmaxLeft = bmaxRight = bmax;

    for (int i = 0; i < bins->binSize - 1; ++i) {
      left += bins->bin[0 * (3 * bins->binSize) + j * bins->binSize + i];
      right -= bins->bin[1 * (3 * bins->binSize) + j * bins->binSize + i];

      assert(left <= numTriangles);
      assert(right <= numTriangles);

      //
      // Split pos bmin + (i + 1) * (bsize / BIN_SIZE)
      // +1 for i since we want a position on right side of the cell.
      //

      pos = bmin[j] + (i + 0.5) * bstep[j];
      bmaxLeft[j] = pos;
      bminRight[j] = pos;

      saLeft = CalculateSurfaceArea(bminLeft, bmaxLeft);
      saRight = CalculateSurfaceArea(bminRight, bmaxRight);

      real cost =
          SAH(left, saLeft, right, saRight, invSaTotal, costTaabb, costTtri);
      if (cost < minCost[j]) {
        //
        // Update the min cost
        //
        minCost[j] = cost;
        minCostPos = pos;
        //minCostAxis = j;
      }
    }

    cutPos[j] = minCostPos;
  }

  //cutAxis = minCostAxis;
  //cutPos = minCostPos;

  // Find min cost axis
  real cost = minCost[0];
  minCostAxis = 0;
  if (cost > minCost[1]) {
    minCostAxis = 1;
    cost = minCost[1];
  }
  if (cost > minCost[2]) {
    minCostAxis = 2;
    cost = minCost[2];
  }

  return true;
}

class SAHPred : public std::unary_function<unsigned int, bool> {
public:
  SAHPred(int axis, real pos, const Mesh *mesh)
      : axis_(axis), pos_(pos), mesh_(mesh) {}

  bool operator()(unsigned int i) const {
    int axis = axis_;
    real pos = pos_;

    unsigned int i0 = mesh_->faces[3 * i + 0];
    unsigned int i1 = mesh_->faces[3 * i + 1];
    unsigned int i2 = mesh_->faces[3 * i + 2];

    real3 p0(&mesh_->vertices[3 * i0]);
    real3 p1(&mesh_->vertices[3 * i1]);
    real3 p2(&mesh_->vertices[3 * i2]);

    real center = p0[axis] + p1[axis] + p2[axis];

    return (center < pos * 3.0);
  }

private:
  int axis_;
  real pos_;
  const Mesh *mesh_;
};

static void ComputeBoundingBox(real3 &bmin, real3 &bmax, real *vertices,
                               unsigned int *faces, unsigned int *indices,
                               unsigned int leftIndex,
                               unsigned int rightIndex) {
  const real kEPS = std::numeric_limits<real>::epsilon() * 1024;

  size_t i = leftIndex;
  size_t idx = indices[i];
  bmin[0] = vertices[3 * faces[3 * idx + 0] + 0] - kEPS;
  bmin[1] = vertices[3 * faces[3 * idx + 0] + 1] - kEPS;
  bmin[2] = vertices[3 * faces[3 * idx + 0] + 2] - kEPS;
  bmax[0] = vertices[3 * faces[3 * idx + 0] + 0] + kEPS;
  bmax[1] = vertices[3 * faces[3 * idx + 0] + 1] + kEPS;
  bmax[2] = vertices[3 * faces[3 * idx + 0] + 2] + kEPS;

  // Assume mesh are composed of all triangles
  for (i = leftIndex; i < rightIndex; i++) { // for each faces
    size_t idx = indices[i];
    for (int j = 0; j < 3; j++) { // for each face vertex
      size_t fid = faces[3 * idx + j];
      for (int k = 0; k < 3; k++) { // xyz
        real minval = vertices[3 * fid + k] - kEPS;
        real maxval = vertices[3 * fid + k] + kEPS;
        if (bmin[k] > minval)
          bmin[k] = minval;
        if (bmax[k] < maxval)
          bmax[k] = maxval;
      }
    }
  }
}

//
// --
//

size_t BVHAccel::BuildTree(const Mesh *mesh, unsigned int leftIdx,
                           unsigned int rightIdx, int depth) {
  assert(leftIdx <= rightIdx);

  debug("d: %d, l: %d, r: %d\n", depth, leftIdx, rightIdx);

  size_t offset = nodes_.size();

  if (stats_.maxTreeDepth < depth) {
    stats_.maxTreeDepth = depth;
  }

  real3 bmin, bmax;
  ComputeBoundingBox(bmin, bmax, mesh->vertices, mesh->faces, &indices_.at(0),
                     leftIdx, rightIdx);

  debug(" bmin = %f, %f, %f\n", bmin[0], bmin[1], bmin[2]);
  debug(" bmax = %f, %f, %f\n", bmax[0], bmax[1], bmax[2]);

  size_t n = rightIdx - leftIdx;
  if ((n < options_.minLeafPrimitives) || (depth >= options_.maxTreeDepth)) {
    // Create leaf node.
    BVHNode leaf;

    leaf.bmin[0] = bmin[0];
    leaf.bmin[1] = bmin[1];
    leaf.bmin[2] = bmin[2];

    leaf.bmax[0] = bmax[0];
    leaf.bmax[1] = bmax[1];
    leaf.bmax[2] = bmax[2];

    assert(leftIdx < std::numeric_limits<unsigned int>::max());

    leaf.flag = 1; // leaf
    leaf.data[0] = n;
    leaf.data[1] = (unsigned int) leftIdx;
    debug(" leaf n = %d, offt = %d\n", n, leftIdx);

    nodes_.push_back(leaf);

    stats_.numLeafNodes++;

    return offset;
  }

  //
  // Create branch node.
  //

  //
  // Compute SAH and find best split axis and position
  //
  int minCutAxis = 0;
  real cutPos[3] = { 0.0, 0.0, 0.0 };

  BinBuffer bins(options_.binSize);
  ContributeBinBuffer(&bins, bmin, bmax, mesh, &indices_.at(0), leftIdx,
                      rightIdx);
  FindCutFromBinBuffer(cutPos, minCutAxis, &bins, bmin, bmax, n,
                       options_.costTaabb);

  debug("depth: %d, cutPos: (%f, %f, %f), cutAxis: %d\n", depth, cutPos[0],
        cutPos[1], cutPos[2], minCutAxis);

  // Try all 3 axis until good cut position avaiable.
  unsigned int midIdx;
  int cutAxis = minCutAxis;
  for (int axisTry = 0; axisTry < 1; axisTry++) {

    unsigned int *begin = &indices_[leftIdx];
    unsigned int *end = &indices_[rightIdx];
    unsigned int *mid = 0;

    // try minCutAxis first.
    cutAxis = (minCutAxis + axisTry) % 3;

    //
    // Split at (cutAxis, cutPos)
    // indices_ will be modified.
    //
    mid = std::partition(begin, end, SAHPred(cutAxis, cutPos[cutAxis], mesh));

    midIdx = leftIdx + (mid - begin);
    if ((midIdx == leftIdx) || (midIdx == rightIdx)) {

      // Can't split well.
      // Switch to object median(which may create unoptimized tree, but stable)
      midIdx = leftIdx + (n >> 1);

      // Try another axis if there's axis to try.

    } else {

      // Found good cut. exit loop.
      break;

    }
  }

  BVHNode node;
  node.axis = cutAxis;
  node.flag = 0; // 0 = branch
  nodes_.push_back(node);

  // Recurively split tree.
  unsigned int leftChildIndex = BuildTree(mesh, leftIdx, midIdx, depth + 1);
  unsigned int rightChildIndex = BuildTree(mesh, midIdx, rightIdx, depth + 1);

  nodes_[offset].data[0] = leftChildIndex;
  nodes_[offset].data[1] = rightChildIndex;

  nodes_[offset].bmin[0] = bmin[0];
  nodes_[offset].bmin[1] = bmin[1];
  nodes_[offset].bmin[2] = bmin[2];

  nodes_[offset].bmax[0] = bmax[0];
  nodes_[offset].bmax[1] = bmax[1];
  nodes_[offset].bmax[2] = bmax[2];

  stats_.numBranchNodes++;

  return offset;

}

bool BVHAccel::Build(const Mesh *mesh, const BVHBuildOptions &options) {
  options_ = options;
  stats_ = BVHBuildStatistics();

  assert(options_.binSize > 1);

  assert(mesh);

  size_t n = mesh->numFaces;
  trace("[BVHAccel] Input # of vertices = %lu\n", mesh->numVertices);
  trace("[BVHAccel] Input # of faces    = %lu\n", mesh->numFaces);

  //
  // 1. Create triangle indices(this will be permutated in BuildTree)
  //
  indices_.resize(n);
  for (size_t i = 0; i < n; i++) {
    indices_[i] = i;
  }

  //
  // 2. Build tree
  //
  BuildTree(mesh, 0, n, 0);

  // Tree will be null if input triangle count == 0.
  if (!nodes_.empty()) {
    // 0 = root node.
    real3 bmin(&nodes_[0].bmin[0]);
    real3 bmax(&nodes_[0].bmax[0]);
    trace("[BVHAccel] bound min = (%f, %f, %f)\n", bmin[0], bmin[1], bmin[2]);
    trace("[BVHAccel] bound max = (%f, %f, %f)\n", bmax[0], bmax[1], bmax[2]);
  }

  trace("[BVHAccel] # of nodes = %lu\n", nodes_.size());

  return true;
}

bool BVHAccel::Dump(const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "[BVHAccel] Cannot write a file: %s\n", filename);
    return false;
  }

  unsigned long long numNodes = nodes_.size();
  assert(nodes_.size() > 0);

  unsigned long long numIndices = indices_.size();

  int r = 0;
  r = fwrite(&numNodes, sizeof(unsigned long long), 1, fp);
  assert(r == 1);

  r = fwrite(&nodes_.at(0), sizeof(BVHNode), numNodes, fp);
  assert(r == numNodes);

  r = fwrite(&numIndices, sizeof(unsigned long long), 1, fp);
  assert(r == 1);

  r = fwrite(&indices_.at(0), sizeof(unsigned int), numIndices, fp);
  assert(r == numIndices);

  fclose(fp);

  return true;
}

bool BVHAccel::Load(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Cannot open file: %s\n", filename);
    return false;
  }

  unsigned long long numNodes;
  unsigned long long numIndices;

  int r = 0;
  r = fread(&numNodes, sizeof(unsigned long long), 1, fp);
  assert(r == 1);
  assert(numNodes > 0);

  nodes_.resize(numNodes);
  r = fread(&nodes_.at(0), sizeof(BVHNode), numNodes, fp);
  assert(r == numNodes);

  r = fread(&numIndices, sizeof(unsigned long long), 1, fp);
  assert(r == 1);

  indices_.resize(numIndices);

  r = fread(&indices_.at(0), sizeof(unsigned int), numIndices, fp);
  assert(r == numIndices);

  fclose(fp);

  return true;
}

namespace {

const int kMaxStackDepth = 512;

inline bool IntersectRayAABB(real &tminOut, // [out]
                             real &tmaxOut, // [out]
                             real maxT, real bmin[3], real bmax[3],
                             real3 rayOrg, real3 rayInvDir, int rayDirSign[3]) {
  real tmin, tmax;

  const real min_x = rayDirSign[0] ? bmax[0] : bmin[0];
  const real min_y = rayDirSign[1] ? bmax[1] : bmin[1];
  const real min_z = rayDirSign[2] ? bmax[2] : bmin[2];
  const real max_x = rayDirSign[0] ? bmin[0] : bmax[0];
  const real max_y = rayDirSign[1] ? bmin[1] : bmax[1];
  const real max_z = rayDirSign[2] ? bmin[2] : bmax[2];

  // X
  const double tmin_x = (min_x - rayOrg[0]) * rayInvDir[0];
  const double tmax_x = (max_x - rayOrg[0]) * rayInvDir[0];

  // Y
  const double tmin_y = (min_y - rayOrg[1]) * rayInvDir[1];
  const double tmax_y = (max_y - rayOrg[1]) * rayInvDir[1];

  tmin = (tmin_x > tmin_y) ? tmin_x : tmin_y;
  tmax = (tmax_x < tmax_y) ? tmax_x : tmax_y;

  // Z
  const double tmin_z = (min_z - rayOrg[2]) * rayInvDir[2];
  const double tmax_z = (max_z - rayOrg[2]) * rayInvDir[2];

  tmin = (tmin > tmin_z) ? tmin : tmin_z;
  tmax = (tmax < tmax_z) ? tmax : tmax_z;

  //
  // Hit include (tmin == tmax) edge case(hit 2D plane).
  //
  if ((tmax > 0.0) && (tmin <= tmax) && (tmin <= maxT)) {

    tminOut = tmin;
    tmaxOut = tmax;

    return true;
  }

  return false; // no hit
}

inline bool TriangleIsect(real &tInOut, real &uOut, real &vOut, const real3 &v0,
                          const real3 &v1, const real3 &v2, const real3 &rayOrg,
                          const real3 &rayDir) {
  const real kEPS = std::numeric_limits<real>::epsilon() * 1024;

  real3 p0(v0[0], v0[1], v0[2]);
  real3 p1(v1[0], v1[1], v1[2]);
  real3 p2(v2[0], v2[1], v2[2]);
  real3 e1, e2;
  real3 p, s, q;

  e1 = p1 - p0;
  e2 = p2 - p0;

  p = vcross(rayDir, e2);

  real invDet;
  real det = vdot(e1, p);
  if (std::abs(det) < kEPS) { // no-cull
    return false;
  }

  invDet = 1.0 / det;

  s = rayOrg - p0;
  q = vcross(s, e1);

  real u = vdot(s, p) * invDet;
  real v = vdot(q, rayDir) * invDet;
  real t = vdot(e2, q) * invDet;

  if (u < 0.0 || u > 1.0)
    return false;
  if (v < 0.0 || u + v > 1.0)
    return false;
  if (t < 0.0 || t > tInOut)
    return false;

  tInOut = t;
  uOut = u;
  vOut = v;

  return true;
}

bool TestLeafNode(Intersection &isect, // [inout]
                  const BVHNode &node, const std::vector<unsigned int> &indices,
                  const Mesh *mesh, const Ray &ray) {
  bool hit = false;

  unsigned int numTriangles = node.data[0];
  unsigned int offset = node.data[1];

  real t = isect.t; // current hit distance

  real3 rayOrg;
  rayOrg[0] = ray.org[0];
  rayOrg[1] = ray.org[1];
  rayOrg[2] = ray.org[2];

  real3 rayDir;
  rayDir[0] = ray.dir[0];
  rayDir[1] = ray.dir[1];
  rayDir[2] = ray.dir[2];

  for (unsigned int i = 0; i < numTriangles; i++) {
    int faceIdx = indices[i + offset];

    int f0 = mesh->faces[3 * faceIdx + 0];
    int f1 = mesh->faces[3 * faceIdx + 1];
    int f2 = mesh->faces[3 * faceIdx + 2];

    real3 v0, v1, v2;
    v0[0] = mesh->vertices[3 * f0 + 0];
    v0[1] = mesh->vertices[3 * f0 + 1];
    v0[2] = mesh->vertices[3 * f0 + 2];

    v1[0] = mesh->vertices[3 * f1 + 0];
    v1[1] = mesh->vertices[3 * f1 + 1];
    v1[2] = mesh->vertices[3 * f1 + 2];

    v2[0] = mesh->vertices[3 * f2 + 0];
    v2[1] = mesh->vertices[3 * f2 + 1];
    v2[2] = mesh->vertices[3 * f2 + 2];

    real u, v;
    if (TriangleIsect(t, u, v, v0, v1, v2, rayOrg, rayDir)) {
      // Update isect state
      isect.t = t;
      isect.u = u;
      isect.v = v;
      isect.faceID = faceIdx;
      hit = true;
    }
  }

  return hit;
}

void BuildIntersection(Intersection &isect, const Mesh *mesh, Ray &ray) {
  // face index
  const unsigned int *faces = mesh->faces;
  const real *vertices = mesh->vertices;
  isect.f0 = faces[3 * isect.faceID + 0];
  isect.f1 = faces[3 * isect.faceID + 1];
  isect.f2 = faces[3 * isect.faceID + 2];

  real3 p0, p1, p2;
  p0[0] = vertices[3 * isect.f0 + 0];
  p0[1] = vertices[3 * isect.f0 + 1];
  p0[2] = vertices[3 * isect.f0 + 2];
  p1[0] = vertices[3 * isect.f1 + 0];
  p1[1] = vertices[3 * isect.f1 + 1];
  p1[2] = vertices[3 * isect.f1 + 2];
  p2[0] = vertices[3 * isect.f2 + 0];
  p2[1] = vertices[3 * isect.f2 + 1];
  p2[2] = vertices[3 * isect.f2 + 2];

  // calc shading point.
  isect.position[0] = ray.org[0] + isect.t * ray.dir[0];
  isect.position[1] = ray.org[1] + isect.t * ray.dir[1];
  isect.position[2] = ray.org[2] + isect.t * ray.dir[2];

  // calc geometric normal.
  real3 p10 = p1 - p0;
  real3 p20 = p2 - p0;
  real3 n = vcross(p10, p20);
  n.normalize();

  isect.geometricNormal = n;
  isect.normal = n;
}

} // namespace

bool BVHAccel::Traverse(Intersection &isect, const Mesh *mesh, Ray &ray) {
  real hitT = std::numeric_limits<real>::max(); // far = no hit.

  int nodeStackIndex = 0;
  std::vector<int> nodeStack(512);
  nodeStack[0] = 0;

  bool ret = false;

  // Init isect info as no hit
  isect.t = hitT;
  isect.u = 0.0;
  isect.v = 0.0;
  isect.faceID = -1;

  int dirSign[3];
  dirSign[0] = ray.dir[0] < 0.0 ? 1 : 0;
  dirSign[1] = ray.dir[1] < 0.0 ? 1 : 0;
  dirSign[2] = ray.dir[2] < 0.0 ? 1 : 0;

  // @fixme { Check edge case; i.e., 1/0 }
  real3 rayInvDir;
  rayInvDir[0] = 1.0 / ray.dir[0];
  rayInvDir[1] = 1.0 / ray.dir[1];
  rayInvDir[2] = 1.0 / ray.dir[2];

  real3 rayOrg;
  rayOrg[0] = ray.org[0];
  rayOrg[1] = ray.org[1];
  rayOrg[2] = ray.org[2];

  real minT, maxT;
  while (nodeStackIndex >= 0) {
    int index = nodeStack[nodeStackIndex];
    BVHNode &node = nodes_[index];

    nodeStackIndex--;

    bool hit = IntersectRayAABB(minT, maxT, hitT, node.bmin, node.bmax, rayOrg,
                                rayInvDir, dirSign);

    if (node.flag == 0) { // branch node

      if (hit) {

        int orderNear = dirSign[node.axis];
        int orderFar = 1 - orderNear;

        // Traverse near first.
        nodeStack[++nodeStackIndex] = node.data[orderFar];
        nodeStack[++nodeStackIndex] = node.data[orderNear];

      }

    } else { // leaf node

      if (hit) {
        if (TestLeafNode(isect, node, indices_, mesh, ray)) {
          hitT = isect.t;
        }
      }

    }
  }

  assert(nodeStackIndex < kMaxStackDepth);

  if (isect.t < std::numeric_limits<real>::max()) {
    BuildIntersection(isect, mesh, ray);
    return true;
  }

  return false;
}
