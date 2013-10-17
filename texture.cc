#include <iostream>

#ifdef ENABLE_PTEX
#include <Ptexture.h>
#endif

#ifdef ENABLE_PTEX
PtexCache*
InitPtex()
{
  int maxMem = 1024*1024;
  PtexCache* c = PtexCache::create(0, maxMem);

  return c;
}

PtexTexture*
LoadPtex(
  PtexCache* cache,
  const char* filename)
{
  Ptex::String err;
  PtexTexture* r = PtexTexture::open(filename, err, /* premult */ 0);

  printf("[Mallie] PtexTexture: %p\n", r);

  if (!r) {
    std::cerr << err.c_str() << std::endl;
    return NULL;
  }

  return r;
}
#endif
