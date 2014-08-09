#include <string>
#include <iostream>
#include <cassert>
#include <cstdio>

#include "common.h"
#include "tiny_obj_loader.h"
#include "mesh_loader.h"
#include "eson.h"

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
  mesh.materialIDs = new unsigned int[numFaces];

  // @todo {}
  mesh.facevarying_normals = NULL;
  mesh.facevarying_uvs = NULL;
  mesh.facevarying_tangents = NULL;
  mesh.facevarying_binormals = NULL;

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

bool
MeshLoader::LoadESON(
  Mesh& mesh,
  const char* filename)
{
  // @todo { Use mmap() }
  std::vector<uint8_t> buf;

  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to load file: %s\n", filename);
  }

  fseek(fp, 0, SEEK_END);
  size_t len = ftell(fp);
  rewind(fp);
  buf.resize(len);
  len = fread(&buf[0], 1, len, fp);
  fclose(fp);

  eson::Value v;

  std::string err = eson::Parse(v, &buf[0]);
  if (!err.empty()) {
    std::cout << "Err: " << err << std::endl;
    exit(1);
  }

  //std::cout << "[LoadESON] # of shapes in .obj : " << shapes.size() << std::endl;


  int64_t num_vertices = v.Get("num_vertices").Get<int64_t>();
  int64_t num_faces = v.Get("num_faces").Get<int64_t>();
  printf("# of vertices: %lld\n", num_vertices);
  printf("# of faces   : %lld\n", num_faces);

  eson::Binary vertices_data = v.Get("vertices").Get<eson::Binary>();
  const float* vertices = reinterpret_cast<float*>(const_cast<uint8_t*>(vertices_data.ptr));

  eson::Binary faces_data = v.Get("faces").Get<eson::Binary>();
  const int* faces = reinterpret_cast<int*>(const_cast<uint8_t*>(faces_data.ptr));

  const float* facevarying_normals = NULL;
  if (v.Has("facevarying_normals")) {
    eson::Binary facevarying_normals_data = v.Get("facevarying_uvs").Get<eson::Binary>();
    facevarying_normals = reinterpret_cast<float*>(const_cast<uint8_t*>(facevarying_normals_data.ptr));
  }

  const float* facevarying_uvs = NULL;
  if (v.Has("facevarying_uvs")) {
    eson::Binary facevarying_uvs_data = v.Get("facevarying_uvs").Get<eson::Binary>();
    facevarying_uvs = reinterpret_cast<float*>(const_cast<uint8_t*>(facevarying_uvs_data.ptr));
  }

  const unsigned short* material_ids = NULL;
  if (v.Has("material_ids")) {
    eson::Binary material_ids_data = v.Get("material_ids").Get<eson::Binary>();
    material_ids = reinterpret_cast<unsigned short*>(const_cast<uint8_t*>(material_ids_data.ptr));
  }


  // ESON -> Mesh
  mesh.numFaces     = num_faces;
  mesh.numVertices  = num_vertices;
  mesh.vertices = new real[num_vertices * 3];
  mesh.faces    = new unsigned int[num_faces * 3];
  mesh.materialIDs = new unsigned int[num_faces];

  for (size_t i = 0; i < 3*num_vertices; i++) {
    mesh.vertices[i] = vertices[i];
  }

  for (size_t i = 0; i < 3*num_faces; i++) {
    mesh.faces[i] = faces[i];
  }

  if (material_ids) {
    for (size_t i = 0; i < num_faces; i++) {
      mesh.materialIDs[i] = material_ids[i];
    }
  } else {
    for (size_t i = 0; i < num_faces; i++) {
      mesh.materialIDs[i] = 0; // 0 = default material.
    }
  }
    
  // @todo {}
  mesh.facevarying_normals = NULL;
  mesh.facevarying_uvs = NULL;
  mesh.facevarying_tangents = NULL;
  mesh.facevarying_binormals = NULL;

  return true;
}
