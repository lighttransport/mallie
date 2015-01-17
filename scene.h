#ifndef __MALLIE_SCENE_H__
#define __MALLIE_SCENE_H__

#include <cassert>

#include "bvh_accel.h"
#include "material.h"

#ifdef ENABLE_EMBREE
#include "embree2/rtcore.h"
#endif

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
  Scene();
  ~Scene();

  // Fixme.
  bool Init(const std::string &objFilename, const std::string &esonFilename,
            const std::string &magicaVoxelFilename,
            const std::string &materialFilename, double sceneScale = 1.0, bool sceneFit = false);

  bool Trace(Intersection &isect, Ray &ray);

  void BoundingBox(real3 &bmin, real3 &bmax);

  real3 GetBackgroundRadiance(real3 &dir);

  const Material &GetMaterial(int matID) const {
	static Material s_default_aterial;
	if (matID < materials_.size()) {
    	return materials_[matID];
	} else {
		return s_default_aterial;
	}
  }

protected:
  Mesh mesh_;
  std::vector<Material> materials_;
#ifdef ENABLE_EMBREE
  RTCScene scene_;
  real3 bmin_;
  real3 bmax_;
#else
  BVHAccel accel_;
#endif
};
}

#endif // __MALLIE_SCENE_H__
