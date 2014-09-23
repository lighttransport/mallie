#ifndef __MAGICAVOXEL_LOADER_H__
#define __MAGICAVOXEL_LOADER_H__

#include <vector>
#include "material.h"

class MagicaVoxelLoader {
public:
  MagicaVoxelLoader() {};
  ~MagicaVoxelLoader() {};

  bool Load(const char *filename, std::vector<Material> &materials,
            std::vector<unsigned char> &voxelData, int &sizeX, int &sizeY,
            int &sizeZ);

private:
};

#endif // __MAGICAVOXEL_LOADER_H__
