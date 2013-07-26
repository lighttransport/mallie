#ifndef __COMMON_H__
#define __COMMON_H__

//typedef double real;
typedef float real;

struct real3 {
    real3() { }
    real3(real xx, real yy, real zz) { x = xx; y = yy; z = zz; }
    real3(real* p) { x = p[0]; y = p[1]; z = p[2]; }

    real3 operator*(real f) const { return real3(x*f, y*f, z*f); }
    real3 operator-(const real3 &f2) const {
        return real3(x-f2.x, y-f2.y, z-f2.z);
    }
    real3 operator*(const real3 &f2) const {
        return real3(x*f2.x, y*f2.y, z*f2.z);
    }
    real3 operator+(const real3 &f2) const {
        return real3(x+f2.x, y+f2.y, z+f2.z);
    }
    real3 operator/(const real3 &f2) const {
        return real3(x/f2.x, y/f2.y, z/f2.z);
    }
    real operator[](int i) const { return (&x)[i]; }
    real &operator[](int i) { return (&x)[i]; }

    real x, y, z;
    //real pad;  // for alignment 
};

struct Ray
{
    real3 org;
    real3 dir;
    real3 invDir;
    int dirSign[3];
};

#endif  // __COMMON_H__
