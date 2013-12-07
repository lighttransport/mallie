#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>

#include "camera.h"
#include "matrix.h"
#include "trackball.h"

using namespace mallie;

static inline double vdot(double a[3], double b[3]) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline void vcross(double c[3], double a[3], double b[3]) {
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

static inline double vlength(double v[3]) {
  double len2 = vdot(v, v);
  if (std::abs(len2) > 1.0e-30) {
    return sqrt(len2);
  }
  return 0.0;
}

static void vnormalize(double v[3]) {
  float len = vlength(v);
  if (std::abs(len) > 1.0e-30) {
    double inv_len = 1.0 / len;
    v[0] *= inv_len;
    v[1] *= inv_len;
    v[2] *= inv_len;
  }
}

void Camera::BuildCameraFrame(double origin[3], double corner[3], double u[3],
                              double v[3], double fov, double quat[4],
                              int width, int height) {
  double e[4][4];
  //printf("--in\n");
  //printf("eye: %f, %f, %f\n", eye_[0], eye_[1], eye_[2]);
  //printf("lookat: %f, %f, %f\n", lookat_[0], lookat_[1], lookat_[2]);
  //printf("up: %f, %f, %f\n", up_[0], up_[1], up_[2]);
  Matrix::LookAt(e, eye_, lookat_, up_);
  //printf("e ---\n");
  //Matrix::Print(e);

  double r[4][4];
  build_rotmatrix(r, quat);
  //printf("m ---\n");
  //Matrix::Print(m);

  double lo[3];
  lo[0] = lookat_[0] - eye_[0];
  lo[1] = lookat_[1] - eye_[1];
  lo[2] = lookat_[2] - eye_[2];
  double dist = vlength(lo);

  double dir[3];
  dir[0] = 0.0;
  dir[1] = 0.0;
  dir[2] = dist;

  Matrix::Inverse(r);
  //printf("r ---\n");
  //Matrix::Print(r);

  double rr[4][4];
  double re[4][4];
  double zero[3] = { 0.0f, 0.0f, 0.0f };
  double localUp[3] = { 0.0f, 1.0f, 0.0f };
  Matrix::LookAt(re, dir, zero, localUp);

  // translate
  re[3][0] += 0.0; // lo[0];
  re[3][1] += 0.0; //lo[1];
  re[3][2] += (eye_[2] - dist);

  // rot -> trans
  //Matrix::Mult(rr, re, r);
  Matrix::Mult(rr, r, re);

  // trans -> rot
  //Matrix::Mult(rr, r, re);
  //printf("re ---\n");
  //Matrix::Print(re);
  //printf("rr ---\n");
  //Matrix::Print(rr);

  double m[4][4];
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      m[j][i] = rr[j][i];
    }
  }

  // translate
  //m[3][0] += 0.0; // lo[0];
  //m[3][1] += 0.0; //lo[1];
  //m[3][2] += (eye_[2] - dist);

  //Matrix::Mult(m, e, rr);
  //Matrix::Mult(m, e, rr);
  //Matrix::Mult(m, r, e);
  //printf("m ---\n");
  //Matrix::Print(m);

  //Matrix::Inverse(m);

  double vzero[3] = { 0.0f, 0.0f, 0.0f };
  double eye1[3];
  Matrix::MultV(eye1, m, vzero);
  //printf("eye  = %f, %f, %f\n", eye_[0], eye_[1], eye_[2]);
  //printf("eye1 = %f, %f, %f\n", eye1[0], eye1[1], eye1[2]);

  double lookat1[3];
  dir[2] = -dir[2];
  Matrix::MultV(lookat1, m, dir);
  //printf("dist    = %f\n", dist);
  //printf("lookat1 = %f, %f, %f\n", lookat1[0], lookat1[1], lookat1[2]);

  double up1[3];
  //printf("up = %f, %f, %f\n", up[0], up[1], up[2]);
  //Matrix::Print(m);
  Matrix::MultV(up1, m, up_);
  //printf("eye1 = %f, %f, %f\n", eye1[0], eye1[1], eye1[2]);
  //printf("up1 = %f, %f, %f\n", up1[0], up1[1], up1[2]);

  // absolute -> relative
  up1[0] -= eye1[0];
  up1[1] -= eye1[1];
  up1[2] -= eye1[2];
  //printf("up1(after) = %f, %f, %f\n", up1[0], up1[1], up1[2]);

  // Use original up vector
  up1[0] = up_[0];
  up1[1] = up_[1];
  up1[2] = up_[2];

#if 0
  float vdir[3];
  vdir[0] = lookat1[0] - eye1[0];
  vdir[1] = lookat1[1] - eye1[1];
  vdir[2] = lookat1[2] - eye1[2];
  double vlen = sqrt(vdir[0] * vdir[0] + vdir[1] * vdir[1] + vdir[2] * vdir[2]);
  double inv_vlen = vlen;
  if (vlen > 1.0e-30) {
    inv_vlen = 1.0 / vlen;
  }
  vdir[0] *= inv_vlen;
  vdir[1] *= inv_vlen;
  vdir[2] *= inv_vlen;
  double up_dot_dir = up1[0] * vdir[0] + up1[1] * vdir[1] + up1[2] * vdir[2];
  if (std::abs(up_dot_dir) > 0.999) {
    // flop y and z
    float tmp = up1[1];
    up1[1] = up1[2];
    up1[2] = -tmp;
  }
#endif

  //glrsSetCamera(&eye1[0], &lookat1[0], &up1[0], 45.0f);
  //fprintf(stderr, "eye: %f %f %f\n", eye1[0], eye1[1], eye1[2]);
  //fprintf(stderr, "lookat: %f %f %f\n", lookat1[0], lookat1[1], lookat1[2]);
  //fprintf(stderr, "up: %f %f %f\n", up1[0], up1[1], up1[2]);
  //fprintf(stderr, "quat: %f %f %f %f\n", quat[0], quat[1], quat[2], quat[3]);

  {
    double flen =
        (0.5f * (double) height / tanf(0.5f * (double)(fov * M_PI / 180.0f)));
    double look1[3];
    look1[0] = lookat1[0] - eye1[0];
    look1[1] = lookat1[1] - eye1[1];
    look1[2] = lookat1[2] - eye1[2];
    //vcross(u, up1, look1);
    // flip
    vcross(u, look1, up1);
    vnormalize(u);

    vcross(v, look1, u);
    vnormalize(v);

    vnormalize(look1);
    look1[0] = flen * look1[0] + eye1[0];
    look1[1] = flen * look1[1] + eye1[1];
    look1[2] = flen * look1[2] + eye1[2];
    corner[0] = look1[0] - 0.5f * (width * u[0] + height * v[0]);
    corner[1] = look1[1] - 0.5f * (width * u[1] + height * v[1]);
    corner[2] = look1[2] - 0.5f * (width * u[2] + height * v[2]);

    origin[0] = eye1[0];
    origin[1] = eye1[1];
    origin[2] = eye1[2];

    // Store intermediate
    origin_[0] = origin[0];
    origin_[1] = origin[1];
    origin_[2] = origin[2];

    corner_[0] = corner[0];
    corner_[1] = corner[1];
    corner_[2] = corner[2];

    du_[0] = u[0];
    du_[1] = u[1];
    du_[2] = u[2];

    dv_[0] = v[0];
    dv_[1] = v[1];
    dv_[2] = v[2];

    fov_ = fov;

  }

}

Ray Camera::GenerateRay(double u, double v) const {
  real3 dir;

  dir[0] = (corner_[0] + u * du_[0] + v * dv_[0]) - origin_[0];
  dir[1] = (corner_[1] + u * du_[1] + v * dv_[1]) - origin_[1];
  dir[2] = (corner_[2] + u * du_[2] + v * dv_[2]) - origin_[2];
  dir.normalize();

  real3 org;
  org[0] = origin_[0];
  org[1] = origin_[1];
  org[2] = origin_[2];

  Ray ray;
  ray.org = org;
  ray.dir = dir;

  return ray;
}
