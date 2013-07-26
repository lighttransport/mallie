#ifndef __MALLIE_SCENE_H__
#define __MALLIE_SCENE_H__

#include "bvh_accel.h"
#include "material.h"

class
Scene
{
  public:
    Scene();
    ~Scene();

    bool Init(const std::string& objFilename);

  protected:
    
    BVHAccel              accel_;
    std::vector<Material> materials_;

};

#endif // __MALLIE_SCENE_H__
