#include <string>
#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstring>

#include "common.h"
#include "tiny_obj_loader.h"
#include "magicavoxel_loader.h"
#include "mesh_loader.h"
#include "eson.h"

namespace {

void calcNormal(real3& N, real3 v0, real3 v1, real3 v2)
{
  real3 v10 = v1 - v0;
  real3 v20 = v2 - v0;

  N = vcross(v20, v10);
  N.normalize();
}

}

bool MeshLoader::LoadObj(Mesh &mesh, const char *filename) {
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err = tinyobj::LoadObj(shapes, materials, filename);

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
    printf("  shape[%ld].normals: %ld\n", i, shapes[i].mesh.normals.size());
    assert((shapes[i].mesh.normals.size() % 3) == 0);

    numVertices += shapes[i].mesh.positions.size() / 3;
    numFaces += shapes[i].mesh.indices.size() / 3;
  }

  // Shape -> Mesh
  mesh.numFaces = numFaces;
  mesh.numVertices = numVertices;
  mesh.vertices = new real[numVertices * 3];
  mesh.faces = new unsigned int[numFaces * 3];
  mesh.materialIDs = new unsigned int[numFaces];
  memset(mesh.materialIDs, 0, sizeof(int) * numFaces);
  mesh.facevarying_normals = new real[numFaces * 3 * 3];
  mesh.facevarying_uvs = new real[numFaces * 3 * 2];
  memset(mesh.facevarying_uvs, 0, sizeof(real) * 2 * 3 * numFaces);

  // @todo {}
  mesh.facevarying_tangents = NULL;
  mesh.facevarying_binormals = NULL;

  size_t vertexIdxOffset = 0;
  size_t faceIdxOffset = 0;
  for (size_t i = 0; i < shapes.size(); i++) {

    for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
      mesh.faces[3 * (faceIdxOffset + f) + 0] =
          shapes[i].mesh.indices[3 * f + 0];
      mesh.faces[3 * (faceIdxOffset + f) + 1] =
          shapes[i].mesh.indices[3 * f + 1];
      mesh.faces[3 * (faceIdxOffset + f) + 2] =
          shapes[i].mesh.indices[3 * f + 2];

      mesh.faces[3 * (faceIdxOffset + f) + 0] += vertexIdxOffset;
      mesh.faces[3 * (faceIdxOffset + f) + 1] += vertexIdxOffset;
      mesh.faces[3 * (faceIdxOffset + f) + 2] += vertexIdxOffset;

      mesh.materialIDs[faceIdxOffset + f] = shapes[i].mesh.material_ids[f];
    }

    for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
      mesh.vertices[3 * (vertexIdxOffset + v) + 0] =
          shapes[i].mesh.positions[3 * v + 0];
      mesh.vertices[3 * (vertexIdxOffset + v) + 1] =
          shapes[i].mesh.positions[3 * v + 1];
      mesh.vertices[3 * (vertexIdxOffset + v) + 2] =
          shapes[i].mesh.positions[3 * v + 2];
    }

    if (shapes[i].mesh.normals.size() > 0) {
      for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
        int f0, f1, f2;

        f0 = shapes[i].mesh.indices[3*f+0];
        f1 = shapes[i].mesh.indices[3*f+1];
        f2 = shapes[i].mesh.indices[3*f+2];

        real3 n0, n1, n2;

        n0[0] = shapes[i].mesh.normals[3 * f0 + 0];
        n0[1] = shapes[i].mesh.normals[3 * f0 + 1];
        n0[2] = shapes[i].mesh.normals[3 * f0 + 2];

        n1[0] = shapes[i].mesh.normals[3 * f1 + 0];
        n1[1] = shapes[i].mesh.normals[3 * f1 + 1];
        n1[2] = shapes[i].mesh.normals[3 * f1 + 2];

        n2[0] = shapes[i].mesh.normals[3 * f2 + 0];
        n2[1] = shapes[i].mesh.normals[3 * f2 + 1];
        n2[2] = shapes[i].mesh.normals[3 * f2 + 2];

        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 0) + 0] = n0[0];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 0) + 1] = n0[1];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 0) + 2] = n0[2];

        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 1) + 0] = n1[0];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 1) + 1] = n1[1];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 1) + 2] = n1[2];

        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 2) + 0] = n2[0];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 2) + 1] = n2[1];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 2) + 2] = n2[2];
      }
    } else {
      // calc geometric normal
      for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
        real3 v0, v1, v2;

        v0[0] = shapes[i].mesh.positions[3 * (3 * f + 0) + 0];
        v0[1] = shapes[i].mesh.positions[3 * (3 * f + 0) + 1];
        v0[2] = shapes[i].mesh.positions[3 * (3 * f + 0) + 2];

        v1[0] = shapes[i].mesh.positions[3 * (3 * f + 1) + 0];
        v1[1] = shapes[i].mesh.positions[3 * (3 * f + 1) + 1];
        v1[2] = shapes[i].mesh.positions[3 * (3 * f + 1) + 2];

        v2[0] = shapes[i].mesh.positions[3 * (3 * f + 2) + 0];
        v2[1] = shapes[i].mesh.positions[3 * (3 * f + 2) + 1];
        v2[2] = shapes[i].mesh.positions[3 * (3 * f + 2) + 2];

        real3 N;
        calcNormal(N, v0, v1, v2);

        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 0) + 0] = N[0];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 0) + 1] = N[1];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 0) + 2] = N[2];

        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 1) + 0] = N[0];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 1) + 1] = N[1];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 1) + 2] = N[2];

        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 2) + 0] = N[0];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 2) + 1] = N[1];
        mesh.facevarying_normals[3 * (3 * (faceIdxOffset + f) + 2) + 2] = N[2];

      }

    }

    if (shapes[i].mesh.texcoords.size() > 0) {
      for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
        int f0, f1, f2;

        f0 = shapes[i].mesh.indices[3*f+0];
        f1 = shapes[i].mesh.indices[3*f+1];
        f2 = shapes[i].mesh.indices[3*f+2];

        real3 n0, n1, n2;

        n0[0] = shapes[i].mesh.texcoords[2 * f0 + 0];
        n0[1] = shapes[i].mesh.texcoords[2 * f0 + 1];

        n1[0] = shapes[i].mesh.texcoords[2 * f1 + 0];
        n1[1] = shapes[i].mesh.texcoords[2 * f1 + 1];

        n2[0] = shapes[i].mesh.texcoords[2 * f2 + 0];
        n2[1] = shapes[i].mesh.texcoords[2 * f2 + 1];

        mesh.facevarying_uvs[2 * (3 * (faceIdxOffset + f) + 0) + 0] = n0[0];
        mesh.facevarying_uvs[2 * (3 * (faceIdxOffset + f) + 0) + 1] = n0[1];

        mesh.facevarying_uvs[2 * (3 * (faceIdxOffset + f) + 1) + 0] = n1[0];
        mesh.facevarying_uvs[2 * (3 * (faceIdxOffset + f) + 1) + 1] = n1[1];

        mesh.facevarying_uvs[2 * (3 * (faceIdxOffset + f) + 2) + 0] = n2[0];
        mesh.facevarying_uvs[2 * (3 * (faceIdxOffset + f) + 2) + 1] = n2[1];
      }
    }

    vertexIdxOffset += shapes[i].mesh.positions.size() / 3;
    faceIdxOffset += shapes[i].mesh.indices.size() / 3;
  }

  return true;
}

bool MeshLoader::LoadESON(Mesh &mesh, const char *filename) {
  // @todo { Use mmap() }
  std::vector<uint8_t> buf;

  FILE *fp = fopen(filename, "rb");
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

  // std::cout << "[LoadESON] # of shapes in .obj : " << shapes.size() <<
  // std::endl;

  int64_t num_vertices = v.Get("num_vertices").Get<int64_t>();
  int64_t num_faces = v.Get("num_faces").Get<int64_t>();
  printf("# of vertices: %lld\n", num_vertices);
  printf("# of faces   : %lld\n", num_faces);

  eson::Binary vertices_data = v.Get("vertices").Get<eson::Binary>();
  const float *vertices =
      reinterpret_cast<float *>(const_cast<uint8_t *>(vertices_data.ptr));

  eson::Binary faces_data = v.Get("faces").Get<eson::Binary>();
  const int *faces =
      reinterpret_cast<int *>(const_cast<uint8_t *>(faces_data.ptr));

  const float *facevarying_normals = NULL;
  if (v.Has("facevarying_normals")) {
    eson::Binary facevarying_normals_data =
        v.Get("facevarying_uvs").Get<eson::Binary>();
    facevarying_normals = reinterpret_cast<float *>(
        const_cast<uint8_t *>(facevarying_normals_data.ptr));
  }

  const float *facevarying_uvs = NULL;
  if (v.Has("facevarying_uvs")) {
    eson::Binary facevarying_uvs_data =
        v.Get("facevarying_uvs").Get<eson::Binary>();
    facevarying_uvs = reinterpret_cast<float *>(
        const_cast<uint8_t *>(facevarying_uvs_data.ptr));
  }

  const unsigned short *material_ids = NULL;
  if (v.Has("material_ids")) {
    eson::Binary material_ids_data = v.Get("material_ids").Get<eson::Binary>();
    material_ids = reinterpret_cast<unsigned short *>(
        const_cast<uint8_t *>(material_ids_data.ptr));
  }

  // ESON -> Mesh
  mesh.numFaces = num_faces;
  mesh.numVertices = num_vertices;
  mesh.vertices = new real[num_vertices * 3];
  mesh.faces = new unsigned int[num_faces * 3];
  mesh.materialIDs = new unsigned int[num_faces];

  for (size_t i = 0; i < 3 * num_vertices; i++) {
    mesh.vertices[i] = vertices[i];
  }

  for (size_t i = 0; i < 3 * num_faces; i++) {
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

bool MeshLoader::LoadMagicaVoxel(Mesh &mesh, std::vector<Material> &materials,
                                 const char *filename) {
  MagicaVoxelLoader loader;
  std::vector<unsigned char> voxelData;
  int size[3];
  bool ret =
      loader.Load(filename, materials, voxelData, size[0], size[1], size[2]);

  if (!ret) {
    fprintf(stderr, "Failed to load .vox file.\n");
    return false;
  }

  // Simple voxel to polygon conversion.
  float P[8][3] = {
      {-1.0, -1.0, 1.0},
      {-1.0, 1.0, 1.0},
      {1.0, 1.0, 1.0},
      {1.0, -1.0, 1.0},
      {-1.0, -1.0, -1.0},
      {-1.0, 1.0, -1.0},
      {1.0, 1.0, -1.0},
      {1.0, -1.0, -1.0},
  };

  int F[6][4] = {
      {0, 3, 2, 1},
      {2, 3, 7, 6},
      {0, 4, 7, 3},
      {1, 2, 6, 5},
      {4, 5, 6, 7},
      {0, 1, 5, 4},
  };

  int voffset = 0;
  int foffset = 0;
  int numVoxels = voxelData.size() / 4;

  mesh.numFaces = numVoxels * 12;
  mesh.numVertices = numVoxels * 8;
  mesh.vertices = new real[mesh.numVertices * 3];
  mesh.faces = new unsigned int[mesh.numFaces * 3];
  mesh.materialIDs = new unsigned int[mesh.numFaces];

  // not used
  mesh.facevarying_normals = NULL;
  mesh.facevarying_uvs = NULL;
  mesh.facevarying_tangents = NULL;
  mesh.facevarying_binormals = NULL;

  float posOffset[3];
  posOffset[0] = -0.5f * size[0];
  posOffset[1] = -0.5f * size[1];
  posOffset[2] = -0.5f * size[2];

  for (size_t i = 0; i < voxelData.size() / 4; i++) {
    int x = voxelData[4 * i + 0];
    int y = voxelData[4 * i + 1];
    int z = voxelData[4 * i + 2];
    int col = voxelData[4 * i + 3] - 1; // index 0 is not used.

    // vert(Zup -> Yup)
    for (int j = 0; j < 8; j++) {
      mesh.vertices[3 * (voffset + j) + 0] = posOffset[0] + x + 0.5f * P[j][0];
      mesh.vertices[3 * (voffset + j) + 2] =
          posOffset[1] + -(y + 0.5f * P[j][1]);
      mesh.vertices[3 * (voffset + j) + 1] = posOffset[2] + z + 0.5f * P[j][2];
    }

    // face
    for (int f = 0; f < 6; f++) {

      mesh.faces[foffset + 6 * f + 0] = voffset + F[f][0];
      mesh.faces[foffset + 6 * f + 1] = voffset + F[f][1];
      mesh.faces[foffset + 6 * f + 2] = voffset + F[f][2];

      mesh.faces[foffset + 6 * f + 3] = voffset + F[f][0];
      mesh.faces[foffset + 6 * f + 4] = voffset + F[f][2];
      mesh.faces[foffset + 6 * f + 5] = voffset + F[f][3];

      mesh.materialIDs[foffset / 3 + 2 * f + 0] = col;
      mesh.materialIDs[foffset / 3 + 2 * f + 1] = col;
    }

    voffset += 8;
    foffset += 3 * 12;
  }

  return true;
}
