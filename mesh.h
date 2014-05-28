#ifndef __MESH_H__
#define __MESH_H__

#include <cstdio>
#include "common.h"

typedef struct {
  size_t numVertices;
  size_t numFaces;
  real *vertices;              /// [xyz] * numVertices
  real *facevarying_normals;   /// [xyz] * 3(triangle) * numFaces
  real *facevarying_tangents;  /// [xyz] * 3(triangle) * numFaces
  real *facevarying_binormals; /// [xyz] * 3(triangle) * numFaces
  real *facevarying_uvs;       /// [xyz] * 3(triangle) * numFaces
  unsigned int *faces;         /// triangle x numFaces
  unsigned int *materialIDs;   /// index x numFaces
} Mesh;

#endif // __MESH_H__
       // vim:set sw=2 ts=2 expandtab:
