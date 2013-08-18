#ifndef __MALLIE_MATRIX_HPP__
#define __MALLIE_MATRIX_HPP__

namespace mallie {

class
Matrix
{
public:
  Matrix();
  ~Matrix();

  static void Print(float m[4][4]);
  static void LookAt(float m[4][4], float eye[3], float lookat[3], float up[3]);
  static void Inverse(float m[4][4]);
  static void Mult(float dst[4][4], float m0[4][4], float m1[4][4]);
  static void MultV(float dst[3], float m[4][4], float v[3]);
};

}

#endif

// vim:set sw=2 ts=2 expandtab:
