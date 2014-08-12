#ifndef __MALLIE_CAMERA_H__
#define __MALLIE_CAMERA_H__

#include <string>
#include "common.h"

namespace mallie {

class Camera {
public:
  Camera(const double eye[3], const double lookat[3], const double up[3]) {
    eye_[0] = eye[0];
    eye_[1] = eye[1];
    eye_[2] = eye[2];
    up_[0] = up[0];
    up_[1] = up[1];
    up_[2] = up[2];
    lookat_[0] = lookat[0];
    lookat_[1] = lookat[1];
    lookat_[2] = lookat[2];
  }
  ~Camera() {};

  void BuildCameraFrame(double origin[3], double corner[3], double u[3],
                        double v[3], double fov, const double quat[4],
                        int width, int height);

  Ray GenerateRay(double u, double v) const;
  Ray GenerateEnvRay(double u, double v) const;
  Ray GenerateStereoEnvRay(double u, double v) const;

  double eye_[3];
  double up_[3];
  double lookat_[3];

  // In world space
  double origin_[3];
  double corner_[3];
  double du_[3];
  double dv_[3];
  double fov_;

  int height_;
  int width_;
};

} // namespace liina

#endif // __LIINA_CAMERA_H__

// vim:set sw=2 ts=2 expandtab:
