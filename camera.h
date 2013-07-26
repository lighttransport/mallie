#ifndef __MALLIE_CAMERA_H__
#define __MALLIE_CAMERA_H__

#include <string>

namespace mallie {

class Camera
{
 public:
  Camera(const float eye[3], const float lookat[3], const float up[3]) {
    eye_[0] = eye[0]; eye_[1] = eye[1]; eye_[2] = eye[2];
    up_[0] = up[0]; up_[1] = up[1]; up_[2] = up[2];
    lookat_[0] = lookat[0]; lookat_[1] = lookat[1]; lookat_[2] = lookat[2];
  }
  ~Camera() {};

  void BuildCameraFrame(float origin[3], float corner[3], float u[3], float v[3], float fov, float quat[4], int width, int height);

  float eye_[3];
  float up_[3];
  float lookat_[3];
};

}   // namespace liina

#endif  // __LIINA_CAMERA_H__

// vim:set sw=2 ts=2 expandtab:
