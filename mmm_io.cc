#include "mmm_io.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

using namespace mallie;

bool
SaveMMM(
    const char* filename,
    double* data,
    int width,
    int height)
{
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "[MMM] Failed to write file: %s\n", filename);
    return false;
  }

  int n;
  n = fwrite(&width, sizeof(int), 1, fp);
  assert(n == 1);

  n = fwrite(&height, sizeof(int), 1, fp);
  assert(n == 1);

  n = fwrite(data, sizeof(double), width * height, fp);
  assert(n == width * height); 

  fclose(fp);

  return true;
}

bool
LoadMMM(
    double** data,
    int& width,
    int& height,
    const char* filename)
{

  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "[MMM] Failed to read file: %s\n", filename);
    return false;
  }

  int n;
  n = fread(&width, sizeof(int), 1, fp);
  assert(n == 1);

  n = fread(&height, sizeof(int), 1, fp);
  assert(n == 1);

  (*data) = reinterpret_cast<double*>(malloc(sizeof(double) * (size_t)width * (size_t)height));
  n = fread((*data), sizeof(double), width * height, fp);
  assert(n == width * height); 

  fclose(fp);
  return true;

}
