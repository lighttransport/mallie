#ifndef __MALLIE_SCENE_H__
#define __MALLIE_SCENE_H__

#include <cassert>

#include "bvh_accel.h"
#include "material.h"

namespace mallie {

class
Scene
{
  public:
    Scene() {};
    ~Scene();

    bool Init(const std::string& objFilename,
              const std::string& materialFilename,
              double sceneScale = 1.0);

    bool Trace(Intersection& isect, Ray& ray);

    void BoundingBox(real3& bmin, real3& bmax);

    real3 GetBackgroundRadiance(real3& dir);

    const Material& GetMaterial(int matID) const {
      assert(matID < materials_.size());
      return materials_[matID];
    }

  protected:
    
    BVHAccel              accel_;
    Mesh                  mesh_;
    std::vector<Material> materials_;

};

}

#endif // __MALLIE_SCENE_H__
