#include "magicavoxel_loader.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <vector>

bool
MagicaVoxelLoader::Load(const char* filename, std::vector<char>& voxel, int &sizeX, int &sizeY, int &sizeZ)
{
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to load file.\n");
    return false;
  }

  // http://voxel.codeplex.com/wikipage?title=VOX%20Format&referringTitle=Home

  char magic[4];
  fread(magic, 1, 4, fp);
  if (magic[0] == 'V' && magic[1] == 'O' && magic[2] == 'X' && magic[3] == ' ') {
    // ok
  } else {
    fprintf(stderr, "Bad magic number.\n");
    fclose(fp);
    return false;
  }

  int ver;
  fread(&ver, sizeof(int), 1, fp);

  int size[3];

  while (!feof(fp)) {
  
    char id[4];
    int  chunkSize;
    int  childChunkSize;

    fread(id, 1, 4, fp);
    fread(&chunkSize, sizeof(int), 1, fp);
    fread(&childChunkSize, sizeof(int), 1, fp);

    // if (id[0] == 'M' && id[1] == 'A' && id[2] == 'I' && id[3] == 'N') {
    //}  

    if (id[0] == 'S' && id[1] == 'I' && id[2] == 'Z' && id[3] == 'E') {
      fread(size, sizeof(int), 3, fp);
      printf("sz = %d, %d, %d\n", size[0], size[1], size[2]);
      fseek(fp, chunkSize - 4 * 3, SEEK_CUR);
    } else if (id[0] == 'X' && id[1] == 'Y' && id[2] == 'Z' && id[3] == 'I') {

      int numVoxels;
      fread(&numVoxels, sizeof(int), 1, fp);

      // x, y, z, and color index(4 chars)
      voxel.resize(numVoxels * 4);
      fread(&voxel.at(0), 1, numVoxels * 4, fp);
    } else if (id[0] == 'R' && id[1] == 'G' && id[2] == 'B' && id[3] == 'A') {
      assert(0);
    } else {
      fseek(fp, chunkSize, SEEK_CUR);
    }
  }

  fclose(fp);

  sizeX = size[0];
  sizeY = size[1];
  sizeZ = size[2];
 
  return true;
}

