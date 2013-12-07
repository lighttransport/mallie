/*
 * Copyright (C) 2012, Tomas Davidovic (http://www.davidovic.cz)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * (The above is MIT License: http://en.wikipedia.org/wiki/MIT_License)
 */

#ifndef __GLRS_HASHGRID_HXX__
#define __GLRS_HASHGRID_HXX__

#include <vector>
#include <cmath>
#include <cstring>
//#include "math.hxx"

#include "vector3.h"

namespace glrs {

class vector3i {
public:
  vector3i(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
  ~vector3i() {}
  ;

  int x, y, z;
};

class vector2i {
public:
  vector2i() {}
  vector2i(int x_, int y_) : x(x_), y(y_) {}
  ~vector2i() {}
  ;

  int x, y;
};

class HashGrid {
public:
  void Reserve(int aNumCells) { mCellEnds.resize(aNumCells); }

  template <typename tParticle>
  void Build(const std::vector<tParticle> &aParticles, float aRadius) {
    mRadius = aRadius;
    mRadiusSqr = mRadius * mRadius;
    mCellSize = mRadius * 2.f;
    mInvCellSize = 1.f / mCellSize;

    mBBoxMin = vector3(1e36f, 1e36f, 1e36f);
    mBBoxMax = vector3(-1e36f, -1e36f, -1e36f);

    for (size_t i = 0; i < aParticles.size(); i++) {
      const vector3 &pos = aParticles[i].GetPosition();
      for (int j = 0; j < 3; j++) {
        mBBoxMax[j] = std::max(mBBoxMax[j], pos[j]);
        mBBoxMin[j] = std::min(mBBoxMin[j], pos[j]);
      }
    }

    mIndices.resize(aParticles.size());
    memset(&mCellEnds[0], 0, mCellEnds.size() * sizeof(int));

    // set mCellEnds[x] to number of particles within x
    for (size_t i = 0; i < aParticles.size(); i++) {
      const vector3 &pos = aParticles[i].GetPosition();
      mCellEnds[GetCellIndex(pos)]++;
    }

    // run exclusive prefix sum to really get the cell starts
    // mCellEnds[x] is now where the cell starts
    int sum = 0;
    for (size_t i = 0; i < mCellEnds.size(); i++) {
      int temp = mCellEnds[i];
      mCellEnds[i] = sum;
      sum += temp;
    }

    for (size_t i = 0; i < aParticles.size(); i++) {
      const vector3 &pos = aParticles[i].GetPosition();
      const int targetIdx = mCellEnds[GetCellIndex(pos)]++;
      mIndices[targetIdx] = int(i);
    }

    // now mCellEnds[x] points to the index right after the last
    // element of cell x

    //// DEBUG
    //for(size_t i=0; i<aParticles.size(); i++)
    //{
    //    const vector3 &pos  = aParticles[i].GetPosition();
    //    vector2i range = GetCellRange(GetCellIndex(pos));
    //    bool found = false;
    //    for(;range.x < range.y; range.x++)
    //    {
    //        if(mIndices[range.x] == i)
    //            found = true;
    //    }
    //    if(!found)
    //        printf("Error at particle %d\n", i);
    //}
  }

  template <typename tParticle, typename tQuery>
  void Process(const std::vector<tParticle> &aParticles, tQuery &aQuery) {
    const vector3 queryPos = aQuery.GetPosition();

    const vector3 distMin = queryPos - mBBoxMin;
    const vector3 distMax = mBBoxMax - queryPos;
    for (int i = 0; i < 3; i++) {
      if (distMin[i] < 0.f)
        return;
      if (distMax[i] < 0.f)
        return;
    }

    const vector3 cellPt = mInvCellSize * distMin;
    const vector3 coordF(std::floor(cellPt[0]), std::floor(cellPt[1]),
                         std::floor(cellPt[2]));

    const int px = int(coordF[0]);
    const int py = int(coordF[1]);
    const int pz = int(coordF[2]);

    const vector3 fractCoord = cellPt - coordF;

    const int pxo = px + (fractCoord[0] < 0.5f ? -1 : +1);
    const int pyo = py + (fractCoord[1] < 0.5f ? -1 : +1);
    const int pzo = pz + (fractCoord[2] < 0.5f ? -1 : +1);

    int found = 0;

    for (int j = 0; j < 8; j++) {
      vector2i activeRange;
      switch (j) {
      case 0:
        activeRange = GetCellRange(GetCellIndex(vector3i(px, py, pz)));
        break;
      case 1:
        activeRange = GetCellRange(GetCellIndex(vector3i(px, py, pzo)));
        break;
      case 2:
        activeRange = GetCellRange(GetCellIndex(vector3i(px, pyo, pz)));
        break;
      case 3:
        activeRange = GetCellRange(GetCellIndex(vector3i(px, pyo, pzo)));
        break;
      case 4:
        activeRange = GetCellRange(GetCellIndex(vector3i(pxo, py, pz)));
        break;
      case 5:
        activeRange = GetCellRange(GetCellIndex(vector3i(pxo, py, pzo)));
        break;
      case 6:
        activeRange = GetCellRange(GetCellIndex(vector3i(pxo, pyo, pz)));
        break;
      case 7:
        activeRange = GetCellRange(GetCellIndex(vector3i(pxo, pyo, pzo)));
        break;
      }

      for (; activeRange.x < activeRange.y; activeRange.x++) {
        const int particleIndex = mIndices[activeRange.x];
        const tParticle &particle = aParticles[particleIndex];

        const float distSqr =
            (aQuery.GetPosition() - particle.GetPosition()).LenSqr();

        if (distSqr <= mRadiusSqr)
          aQuery.Process(particle);
      }
    }
  }

private:

  vector2i GetCellRange(int aCellIndex) const {
    if (aCellIndex == 0)
      return vector2i(0, mCellEnds[0]);
    return vector2i(mCellEnds[aCellIndex - 1], mCellEnds[aCellIndex]);
  }

  int GetCellIndex(const vector3i &aCoord) const {
    unsigned int x = (unsigned int)(aCoord.x);
    unsigned int y = (unsigned int)(aCoord.y);
    unsigned int z = (unsigned int)(aCoord.z);

    return int(((x * 73856093) ^ (y * 19349663) ^ (z * 83492791)) %
               (unsigned int)(mCellEnds.size()));
  }

  int GetCellIndex(const vector3 &aPoint) const {
    const vector3 distMin = aPoint - mBBoxMin;

    const vector3 coordF(std::floor(mInvCellSize * distMin[0]),
                         std::floor(mInvCellSize * distMin[1]),
                         std::floor(mInvCellSize * distMin[2]));

    const vector3i coordI =
        vector3i(int(coordF[0]), int(coordF[1]), int(coordF[2]));

    return GetCellIndex(coordI);
  }

private:

  vector3 mBBoxMin;
  vector3 mBBoxMax;
  std::vector<int> mIndices;
  std::vector<int> mCellEnds;

  float mRadius;
  float mRadiusSqr;
  float mCellSize;
  float mInvCellSize;
};

} // namespace

#endif //__GLRS_HASHGRID_HXX__
