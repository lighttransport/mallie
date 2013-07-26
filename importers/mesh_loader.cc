#include <string>
#include <iostream>
#include <cassert>
#include <cstdio>

#include "common.h"
#include "tiny_obj_loader.h"
#include "mesh_loader.h"

bool
MeshLoader::LoadObj(
  Mesh& mesh,
  const char* filename)
{
  std::vector<tinyobj::shape_t> shapes;

  std::string err = tinyobj::LoadObj(shapes, filename);

  if (!err.empty()) {
    std::cerr << err << std::endl;
    return false;
  }

  std::cout << "[LoadOBJ] # of shapes in .obj : " << shapes.size() << std::endl;

  size_t numVertices = 0;
  size_t numFaces = 0;
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("  shape[%ld].name = %s\n", i, shapes[i].name.c_str());
    printf("  shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
    assert((shapes[i].mesh.indices.size() % 3) == 0);
    printf("  shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
    assert((shapes[i].mesh.positions.size() % 3) == 0);

    numVertices += shapes[i].mesh.positions.size() / 3;
    numFaces    += shapes[i].mesh.indices.size() / 3;
  }

  // Shape -> Mesh
  mesh.numFaces     = numFaces;
  mesh.numVertices  = numVertices;
  mesh.vertices = new real[numVertices * 3];
  mesh.faces    = new unsigned int[numFaces * 3];

  size_t vertexIdxOffset = 0;
  size_t faceIdxOffset = 0;
  for (size_t i = 0; i < shapes.size(); i++) {

    for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
      mesh.faces[3*(faceIdxOffset+f)+0] = shapes[i].mesh.indices[3*f+0];
      mesh.faces[3*(faceIdxOffset+f)+1] = shapes[i].mesh.indices[3*f+1];
      mesh.faces[3*(faceIdxOffset+f)+2] = shapes[i].mesh.indices[3*f+2];

      mesh.faces[3*(faceIdxOffset+f)+0] += vertexIdxOffset;
      mesh.faces[3*(faceIdxOffset+f)+1] += vertexIdxOffset;
      mesh.faces[3*(faceIdxOffset+f)+2] += vertexIdxOffset;
    }

    for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
      mesh.vertices[3*(vertexIdxOffset+v)+0] = shapes[i].mesh.positions[3*v+0];
      mesh.vertices[3*(vertexIdxOffset+v)+1] = shapes[i].mesh.positions[3*v+1];
      mesh.vertices[3*(vertexIdxOffset+v)+2] = shapes[i].mesh.positions[3*v+2];
    }

    vertexIdxOffset += shapes[i].mesh.positions.size() / 3;
    faceIdxOffset   += shapes[i].mesh.indices.size() / 3;
  }

  return true;
}
