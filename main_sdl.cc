#include "main_sdl.h"

#include <cassert>
#include <vector>
#include <cassert>
#include <iostream>

#ifdef ENABLE_SDL

#ifdef _OPENMP
#include <omp.h>
#endif

#include <SDL.h> // SDL2

#include "trackball.h"
#include "camera.h"
#include "timerutil.h"
#include "render.h"

#define PIXELSTEP_COARSE  (8)

namespace mallie {

//
// -- GUI variables --
//
static bool  gViewChanged    = true; // initial is true.
static bool  gRenderInteractive = false;
static int   gMouseX = -1, gMouseY = -1;
static bool  gMouseMoving = false;
static int   gMouseButton = 0;
static bool  gNeedRedraw = false;
static bool  gShiftPressed = false;
static bool  gCtrlPressed  = false;
static bool  gLightEditing = false;
static float gEye[3] = {0.0f, 0.0f, -5.0f};
static float gUp[3] = {0.0f, 1.0f, 0.0f};
static float gLookat[3] = {0.0f, 0.0f, 0.0f};
static float gFov = 45.0f;
static float gScale = 0.1f;
static float gPrevQuat[4] = {0.0f, 0.0f, 0.0f, 0.0f};
static float gCurrQuat[4] = {0.0f, 0.0f, 0.0f, 0.0f};
static float gInitQuat[4] = {0.0f, 0.0f, 0.0f, 1.0f};
static float gRotate[3] = {0.0f, 0.0f, 0.0f};
static float gOrigin[3], gCorner[3], gDu[3], gDv[3];
static float gIntensity = 1.0f;
static float gTransferOffset = 0.0f;

// Progressive render param
static int   gRenderPixelStep = PIXELSTEP_COARSE;
static int   gRenderPasses = 1;

int gWidth = 256;
int gHeight = 256;

//SDL_Surface* gSurface = NULL;
SDL_Window* gWindow = NULL;
SDL_Surface* gSurface = NULL;
SDL_Renderer* gSDLRenderer = NULL;

std::vector<float> gImage;
std::vector<float> gFramebuffer;  // HDR framebuffer
RenderConfig gRenderConfig;

static void
EulerToQuatRad(
  float quat[4],
  double x, double y, double z) // in radian. yaw, pitch, roll
{
  double rx = x;
  double ry = y;
  double rz = z;

  double hx = 0.5 * rx;
  double hy = 0.5 * ry;
  double hz = 0.5 * rz;

  double cosHx = cos(hx);
  double cosHy = cos(hy);
  double cosHz = cos(hz);

  double sinHx = sin(hx);
  double sinHy = sin(hy);
  double sinHz = sin(hz);

  quat[0] = cosHx * cosHy * cosHz + sinHx * sinHy * sinHz;
  quat[1] = sinHx * cosHy * cosHz - cosHx * sinHy * sinHz;
  quat[2] = cosHx * sinHy * cosHz + sinHx * cosHy * sinHz;
  quat[3] = cosHx * cosHy * sinHz - sinHx * sinHy * cosHz;
}

static void
EulerToQuatZYX(
  float quat[4],
  double x, double y, double z) // in radian. yaw, pitch, roll
{
  double rx = x;
  double ry = y;
  double rz = z;

  double hx = 0.5 * rx;
  double hy = 0.5 * ry;
  double hz = 0.5 * rz;

  double cosHx = cos(hx);
  double cosHy = cos(hy);
  double cosHz = cos(hz);

  double sinHx = sin(hx);
  double sinHy = sin(hy);
  double sinHz = sin(hz);

  quat[0] = cosHx * cosHy * cosHz + sinHx * sinHy * sinHz;
  quat[1] = sinHx * cosHy * cosHz - cosHx * sinHy * sinHz;
  quat[2] = cosHx * sinHy * cosHz + sinHx * cosHy * sinHz;
  quat[3] = cosHx * cosHy * sinHz - sinHx * sinHy * cosHz;
}

static void
AccumImage(
  std::vector<float>& dst,
  const std::vector<float>& src)
{
  assert(dst.size() == src.size());
  for (size_t i = 0; i < src.size(); i++) {
    dst[i] += src[i];
  } 
}
  
static void
ClearImage(
  std::vector<float>& img)
{
  for (size_t i = 0; i < img.size(); i++) {
    img[i] = 0.0f;
  } 
}

inline unsigned char fclamp(float x)
{
  int i = x * 255.5;
  if (i < 0) return 0;
  if (i > 255) return 255;
  return (unsigned char)i;
}

void
SaveCamera(
  const std::string& filename)
{
  FILE* fp = fopen(filename.c_str(), "w");

  fprintf(fp, "%f %f %f\n", gEye[0], gEye[1], gEye[2]);
  fprintf(fp, "%f %f %f\n", gLookat[0], gLookat[1], gLookat[2]);
  fprintf(fp, "%f %f %f %f\n", gCurrQuat[0], gCurrQuat[1], gCurrQuat[2], gCurrQuat[3]);

  fclose(fp);

  std::cout << "[Mallie] Save camera data to: " << filename << std::endl;
}

void
LoadCamera(
  const std::string& filename)
{
  FILE* fp = fopen(filename.c_str(), "r");

  fscanf(fp, "%f %f %f\n", &gEye[0], &gEye[1], &gEye[2]);
  fscanf(fp, "%f %f %f\n", &gLookat[0], &gLookat[1], &gLookat[2]);
  fscanf(fp, "%f %f %f %f\n", &gCurrQuat[0], &gCurrQuat[1], &gCurrQuat[2], &gCurrQuat[3]);

  fclose(fp);
}

void HandleMouseButton(SDL_Event e)
{
    if (e.type == SDL_MOUSEBUTTONUP) {
        gMouseMoving = false;
        gNeedRedraw = true;
        gViewChanged = true;
        gMouseButton = 0;
        gRenderInteractive = false;
        gRenderPasses = 1;
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {

        gMouseX = e.motion.x;
        gMouseY = e.motion.y;
        gMouseMoving = true;
        gRenderInteractive = true;
        gRenderPasses = 1;
        gRenderPixelStep = PIXELSTEP_COARSE;

        if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) {
          gMouseButton = 1; // left
        } else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(2)) {
          gMouseButton = 2; // middle
        } else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3)) {
          gMouseButton = 3; // right
        }
        trackball(gPrevQuat, 0.0, 0.0, 0.0, 0.0);
    }

}

void HandleMouseMotion(SDL_Event e)
{
    float rotScale = 1.0;

    if (gMouseMoving) {

      int x = e.motion.x;
      int y = e.motion.y;

      gViewChanged = true;
      gNeedRedraw = true;
      gRenderInteractive = true;
      gRenderPasses = 1;

      if (gCtrlPressed || (gMouseButton == 3)) {

        gEye[2]      -= gScale * (gMouseY - y);
        gLookat[2]   -= gScale * (gMouseY - y);

      } else if (gShiftPressed || (gMouseButton == 2)) {

        gEye[0]    += gScale * (gMouseX - x);
        gEye[1]    -= gScale * (gMouseY - y);
        gLookat[0] += gScale * (gMouseX - x);
        gLookat[1] -= gScale * (gMouseY - y);

      } else {
        //trackball(gPrevQuat,
        //    0.0 * (2.0f * gMouseX - gWidth) / (float)gWidth,
        //    rotScale * (gHeight - 2.0f * gMouseY) / (float)gHeight,
        //    0.0 * (2.0f * x - gWidth) / (float)gWidth,
        //    rotScale * (gHeight - 2.0f * y) / (float)gHeight);

        //trackball(gPrevQuat,
        //    rotScale * (2.0f * gMouseX - gWidth) / (float)gWidth,
        //    rotScale * (gHeight - 2.0f * gMouseY) / (float)gHeight,
        //    rotScale * (2.0f * x - gWidth) / (float)gWidth,
        //    rotScale * (gHeight - 2.0f * y) / (float)gHeight);
        //trackball(gPrevQuat,
        //    0.0f,
        //    rotScale * (gHeight - 2.0f * gMouseY) / (float)gHeight,
        //    0.0f,
        //    rotScale * (gHeight - 2.0f * y) / (float)gHeight);

        double xx = (x - gMouseX) / (double)gWidth;
        double yy = (y - gMouseY) / (double)gHeight;
        double zz = 0.0;
        //EulerToQuatRad(gPrevQuat, xx, yy, zz);
        //printf("quat = %f, %f, %f, %f\n", gPrevQuat[0], gPrevQuat[1], gPrevQuat[2], gPrevQuat[3]);

        double scale = M_PI * 2.0; // Heuristic value
        gRotate[0] += scale * xx;
        gRotate[1] += scale * yy;
        // clamp
        double eps = 1.0e-3;
        if (gRotate[1] <= -(0.5*M_PI-eps)) gRotate[1] = -0.5*M_PI + eps;
        if (gRotate[1] >=  (0.5*M_PI-eps)) gRotate[1] = 0.5*M_PI -eps;

        add_quats(gPrevQuat, gCurrQuat, gCurrQuat);

      }
    }

    gMouseX = e.motion.x;
    gMouseY = e.motion.y;
}

bool HandleKey(SDL_Event e)
{
  if (e.type == SDL_KEYUP) {
    gShiftPressed = false;
    gCtrlPressed = false;
    gRenderInteractive = false;
    gRenderPasses = 1;
  } else if (e.type == SDL_KEYDOWN) {
    gRenderInteractive = true;
    gRenderPasses = 1;
    gRenderPixelStep = PIXELSTEP_COARSE;
    switch (e.key.keysym.sym) {
      case SDLK_ESCAPE:
      case 'q':
        //exit(-1);
        return true;
        break;
      case SDLK_SPACE:
          // reset rotation
          gEye[0] = gRenderConfig.eye[0];
          gEye[1] = gRenderConfig.eye[1];
          gEye[2] = gRenderConfig.eye[2];
          trackball(gCurrQuat, 0.0f, 0.0f, 0.0f, 0.0f);
          trackball(gPrevQuat, 0.0f, 0.0f, 0.0f, 0.0f);
          gRotate[0] = gRotate[1] = gRotate[2] = 0.0f;
          gNeedRedraw = true;
          break;
      case 'i':
          gIntensity += 0.1f;
          gNeedRedraw = true;
          break;
      case 'o':
          gIntensity -= 0.1f;
          if (gIntensity < 0.1f) {
            gIntensity = 0.1f;
          }
          gNeedRedraw = true;
          break;
      case 'j':
          gTransferOffset -= 0.02f;
          if (gTransferOffset < 0.0f) {
            gTransferOffset = 0.0f;
          }
          gNeedRedraw = true;
          break;
      case 'k':
          gTransferOffset += 0.02f;
          gNeedRedraw = true;
          break;
      case SDLK_LSHIFT:
          gShiftPressed = true;
          break;
      case SDLK_TAB:
      case SDLK_LCTRL:
          gCtrlPressed = true;
          break;
      default:
          break;
    }
  }

  return false;
}

void Display(
  SDL_Surface* surface,
  const std::vector<float>& image,
  int passes,
  int width,
  int height)
{
  SDL_SetRenderDrawColor(gSDLRenderer, 0, 0, 0, 255);
  SDL_RenderClear(gSDLRenderer);

  SDL_LockSurface(surface);

  // ARGB
  unsigned char* data = (unsigned char*)surface->pixels;
  float scale = 1.0f / (float)passes;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {

      unsigned char col[3];
      col[0] = x % 255;
      col[1] = y % 255;
      col[2] = 127;

#ifdef __APPLE__
      // RGBA
      data[4*(y*width+x)+0] = fclamp(scale * image[3*(y*width+x)+0]);
      data[4*(y*width+x)+1] = fclamp(scale * image[3*(y*width+x)+1]);
      data[4*(y*width+x)+2] = fclamp(scale * image[3*(y*width+x)+2]);
      data[4*(y*width+x)+3] = 255;
#else
      // BGRA?
      data[4*(y*width+x)+2] = fclamp(scale * image[3*(y*width+x)+0]);
      data[4*(y*width+x)+1] = fclamp(scale * image[3*(y*width+x)+1]);
      data[4*(y*width+x)+0] = fclamp(scale * image[3*(y*width+x)+2]);
      data[4*(y*width+x)+3] = 255;
#endif
    }
  }

  SDL_UnlockSurface(surface);
  SDL_RenderPresent(gSDLRenderer);
}

Uint32 TimeLeft(int interval)
{
    static Uint32 next_time = 0;
    Uint32 now;

    now = SDL_GetTicks();
    if ( next_time <= now ) {
        next_time = now+interval;
        return(0);
    }

    return(next_time-now);
}

static bool
CheckSDLEvent()
{
  SDL_Event event;
  SDL_PumpEvents();
  //if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_EVENTMASK (SDL_MOUSEBUTTONDOWN) | SDL_EVENTMASK(SDL_KEYDOWN)) > 0) {
  if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_MOUSEBUTTONDOWN | SDL_KEYDOWN, SDL_MOUSEBUTTONDOWN | SDL_KEYDOWN) > 0) {
    return true;
  }
  return false;
}

static void
Init(
  const RenderConfig& config)
{
  // Save
  gRenderConfig = config;

  gFov = config.fov;

  gEye[0] = config.eye[0];
  gEye[1] = config.eye[1];
  gEye[2] = config.eye[2];

  gLookat[0] = config.lookat[0];
  gLookat[1] = config.lookat[1];
  gLookat[2] = config.lookat[2];

  gUp[0] = config.up[0];
  gUp[1] = config.up[1];
  gUp[2] = config.up[2];

  trackball(gCurrQuat, 0.0f, 0.0f, 0.0f, 0.0f);
  add_quats(gInitQuat, gCurrQuat, gCurrQuat);

  Camera camera(gEye, gLookat, gUp);
  camera.BuildCameraFrame(gOrigin, gCorner, gDu, gDv, gFov, gCurrQuat, gWidth, gHeight);
  //printf("[Mallie] eye    = %f, %f, %f\n", gEye[0], gEye[1], gEye[2]);
  //printf("[Mallie] lookat = %f, %f, %f\n", gLookat[0], gLookat[1], gLookat[2]);
  //printf("[Mallie] up     = %f, %f, %f\n", gUp[0], gUp[1], gUp[2]);
}

void
DoMainSDL(
  Scene& scene,
  const RenderConfig& config)
{
  printf("[Mallie] SDL window mode.\n");

  gWidth = config.width;
  gHeight = config.height;

  gWindow = SDL_CreateWindow("Mallie", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gWidth, gHeight, 0);
  if (!gWindow) {
    printf("SDL err: %s\n", SDL_GetError());
    exit(1);
  }

  gSDLRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_SOFTWARE);
  if (!gSDLRenderer) {
    printf("SDL err: %s\n", SDL_GetError());
    exit(1);
  }

  gSurface = SDL_GetWindowSurface(gWindow);
  if (!gSurface) {
    printf("SDL err: %s\n", SDL_GetError());
    exit(1);
  }

  gFramebuffer.resize(gWidth * gHeight * 3); // RGB
  ClearImage(gFramebuffer);

  gImage.resize(gWidth * gHeight * 3); // RGB

  Init(config);

  SDL_Event event;

  bool done = false;
  while(!done) {

    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_QUIT:
          done = true;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          done = HandleKey(event);
          break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
          HandleMouseButton(event);
          break;
        case SDL_MOUSEMOTION:
          HandleMouseMotion(event);
          break;
      }

      if (done) { break; }
    }

    if (done) { break; }

    //SDL_Delay(33);

    if ((gRenderPasses >= config.num_passes)) {
      //printf("Render finished\n");
      // render finished
      //Display(gSurface, gFramebuffer, gRenderPasses, config.width, config.height);
      continue;
    }
      
    // Use Euler rotation.
    //printf("rot = %f, %f, %f\n", 180*gRotate[0]/M_PI, 180*gRotate[1]/M_PI, 180*gRotate[2]/M_PI);
    EulerToQuatRad(gCurrQuat, gRotate[2], gRotate[0], gRotate[1]+M_PI);
    //printf("quat = %f, %f, %f, %f\n", gCurrQuat[0], gCurrQuat[1], gCurrQuat[2], gCurrQuat[3]);

    Render(scene, config, gImage, gEye, gLookat, gUp, gCurrQuat, gRenderPixelStep);

    // Always clar framebuffer for intermediate result
    if (gRenderPixelStep > 1) {
      ClearImage(gFramebuffer);
    }

    AccumImage(gFramebuffer, gImage);

    Display(gSurface, gFramebuffer, gRenderPasses, config.width, config.height);

    //printf("step = %d, interactive = %d\n", gRenderPixelStep, gRenderInteractive);

    // Increment render pass.
    if (!gRenderInteractive && gRenderPixelStep == 1) {
      gRenderPasses++;
    }

    if (!gRenderInteractive) {
      gRenderPixelStep >>= 1;

      if (gRenderPixelStep == 1) {
        ClearImage(gFramebuffer);
      }

      if (gRenderPixelStep < 1) {
        gRenderPixelStep = 1;
      }
    } else {
      gRenderPasses = 1;
    }
    

  }

  printf("\n");
}

}
#else // ENABLE_SDL
namespace mallie {

void
DoMainSDL(
  Scene& scene,
  const RenderConfig& config)
{
  assert(0);

  return;
}

}
#endif  // ENABLE_SDL



