#ifndef __MALLIE_SCENE_H__
#define __MALLIE_SCENE_H__

#include <cassert>

#include "bvh_accel.h"
#include "material.h"

namespace mallie {

typedef enum {
  PRIMITIVE_POLYGON,
} PrimitiveType;

typedef struct {
  PrimitiveType type;
  BVHAccel *accel;
} Primitive;

//< Scene graph node class
class Node {
public:
  Node() {}
  ~Node() {}

  //< Recompute transform matrix(includes child nodes).
  void UpdateTransform();

private:
  double xform_[16];               //< Transform of this node.
  double xformInverse_;            //< Inverse transform
  double xformInverse33_;          //< Upper 3x3 of inverse transform
  double xformInverseTranspose33_; //< Upper 3x3 of inverse transpose transform
  std::vector<Node *> children_;   //< Children
};

//< Scene class. Usually only one instance of Scene class in rendering.
class Scene {
public:
  Scene() {};
  ~Scene();

  // Fixme.
  bool Init(const std::string &objFilename, const std::string &esonFilename,
            const std::string &materialFilename, double sceneScale = 1.0);

  bool Trace(Intersection &isect, Ray &ray);

  void BoundingBox(real3 &bmin, real3 &bmax);

  real3 GetBackgroundRadiance(real3 &dir);

  const Material &GetMaterial(int matID) const {
    assert(matID < materials_.size());
    return materials_[matID];
  }

protected:
  BVHAccel accel_;
  Mesh mesh_;
  std::vector<Material> materials_;
};
}

#endif // __MALLIE_SCENE_H__
