#ifndef __VECTOR3_H__
#define __VECTOR3_H__

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cassert>

namespace glrs {

/**
  * @class vector3
  * @brief vector3
  */
template <class T> class vector3t {
public:
  //-----------------------------------------------
  // type defines
  typedef T value_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef vector3t<T> this_type;

  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  typedef T *iterator;
  typedef const float *const_iterator;

public:
  static const size_type c_size = 3; // container size
public:
  //-----------------------------------------------
  // functions for iterator
  iterator begin() { return element(); }
  iterator end() { return element() + c_size; }
  const_iterator begin() const { return element(); }
  const_iterator end() const { return element() + c_size; }

public:
#define SC_(a) static_cast<float>(a)
  //-----------------------------------------------
  // constructors and destructor
  vector3t() {}

  vector3t(value_type _x, value_type _y, value_type _z)
      : m0(_x), m1(_y), m2(_z) {}

  vector3t(const this_type &rhs) : m0(rhs.m0), m1(rhs.m1), m2(rhs.m2) {}

  explicit vector3t(const value_type rhs[c_size])
      : m0(SC_(rhs[0])), m1(SC_(rhs[1])), m2(SC_(rhs[2])) {}

  template <class X> vector3t(const vector3t<X> &rhs) {
    m0 = SC_(rhs[0]);
    m1 = SC_(rhs[1]);
    m2 = SC_(rhs[2]);
  }

  ~vector3t() {}
  //
  // void swap(this_type& rhs){std::swap(*this,rhs);}

  //-----------------------------------------------
  // inserters

  this_type &operator=(const this_type &rhs) {

    m0 = rhs.m0;
    m1 = rhs.m1;
    m2 = rhs.m2;

    return *this;
  }

  template <class X> this_type &operator=(const vector3t<X> &rhs) {

    m0 = SC_(rhs[0]);
    m1 = SC_(rhs[1]);
    m2 = SC_(rhs[2]);

    return *this;
  }

  void assign(size_type num, value_type val) {
    size_t sz = (num < c_size) ? num : c_size;
    for (size_t i = 0; i < sz; i++)
      element()[i] = val;
  }

#undef SC_
  //-----------------------------------------------
  // operators

  this_type &negate() {
    m0 = -m0;
    m1 = -m1;
    m2 = -m2;
    return *this;
  }

#define DECLARE_OP_EQUAL(OP)                                                   \
  this_type &operator OP(const this_type &rhs) {                               \
    m0 OP rhs.m0;                                                              \
    m1 OP rhs.m1;                                                              \
    m2 OP rhs.m2;                                                              \
    return *this;                                                              \
  }

  DECLARE_OP_EQUAL(+= )
  DECLARE_OP_EQUAL(-= )
  DECLARE_OP_EQUAL(*= )
  DECLARE_OP_EQUAL(/= )

#undef DECLARE_OP_EQUAL

  this_type &operator*=(T rhs) {
    m0 *= rhs;
    m1 *= rhs;
    m2 *= rhs;

    return *this;
  }
  this_type &operator/=(T rhs) {
    m0 /= rhs;
    m1 /= rhs;
    m2 /= rhs;

    return *this;
  }
  //--------------------------------

  value_type &operator[](size_type i) { return element()[i]; }

  value_type operator[](size_type i) const { return element()[i]; }

  value_type &at(size_type i) {
    // if(c_size<=i){throw std::out_of_range(debug());}
    return element()[i];
  }

  const value_type &at(size_type i) const {
    // if(c_size<=i){throw std::out_of_range(debug());}
    return element()[i];
  }

  //-----------------------------------------------
  // utilities
  value_type length() const {
    using namespace std;
    return sqrt(sqr_length());
  }
  value_type sqr_length() const { return ((m0 * m0) + (m1 * m1) + (m2 * m2)); }

  value_type sum() const { return m0 + m1 + m2; }

  this_type &normalize() {
    using namespace std;

    value_type length = sqr_length(); //||V||^2
    // if (length == T()) return *this;

    length = value_type(1) / sqrt(length);
    m0 *= length;
    m1 *= length;
    m2 *= length;

    return *this;
  }

private:
  value_type m0, m1, m2;

  T *element() { return &m0; }
  const T *element() const { return &m0; }
};

//-----------------------------------------------
// utility functions
//-----------------------------------------------
// Not a member!
//-----------------------------------------------

//-----------------------------------------------
// unary operator
/**
         *  @name unary operator
         *  @relates vector3
         */
//@{
template <class T> inline const vector3t<T> operator+(const vector3t<T> &rhs) {
  return rhs;
}
template <class T> inline vector3t<T> operator-(const vector3t<T> &rhs) {
  return vector3t<T>(rhs).negate();
}
//@}

//-----------------------------------------------
// binary operator
/**
         * @name binary operator
         * @relates vector3
         */
//@{

#define DECLARE_OPERATOR(OP)                                                   \
  template <class T>                                                           \
  inline vector3t<T> operator OP(const vector3t<T> &lhs,                       \
                                 const vector3t<T> &rhs) {                     \
    return vector3t<T>(lhs) OP## = rhs;                                        \
  }
DECLARE_OPERATOR(+)
DECLARE_OPERATOR(-)
DECLARE_OPERATOR(*)
DECLARE_OPERATOR(/ )

#undef DECLARE_OPERATOR

//@}

//-----------------------------------------------
// specific scalar
/**
         *	@name specific scalar
         *	@relates vector
         */
//@{

/**
         *	@param lhs an any type.
         *	@param rhs a vector.
         *	@return lhs * rhs
         */
template <class T>
inline vector3t<T> operator*(double lhs, const vector3t<T> &rhs) {
  return vector3t<T>(rhs) *= (T)lhs;
}
/**
         *	@param lhs a vector.
         * 	@param rhs an any type.
         *	@return lhs * rhs
         */
template <class T>
inline vector3t<T> operator*(const vector3t<T> &lhs, double rhs) {
  return vector3t<T>(lhs) *= rhs;
}

/**
         *	@param lhs a vector.
         *	@param rhs an any type.
         *	@return lhs / rhs
         */
template <class T>
inline vector3t<T> operator/(const vector3t<T> &lhs, double rhs) {
  return vector3t<T>(lhs) /= rhs;
}
//@}

//-----------------------------------------------
// utility functions
/**
         *	@name utility
         *	@relates vector
         */
//@{

/**
         *	@param rhs a vector.
         *	@return || rhs ||
         */
template <class T> inline T length(const vector3t<T> &rhs) {
  return rhs.length();
}
/**
         *	@param rhs a vector.
         *	@return || rhs ||^2
         */
template <class T> inline T sqr_length(const vector3t<T> &rhs) {
  return rhs.sqr_length();
}

/**
         *	@param rhs a vector.
         *	@return sigma (rhs)
         */
template <class T> inline T sum(const vector3t<T> &rhs) { return rhs.sum(); }

/**
         *	@param rhs a vector.
         *	@return rhs/|| rhs ||
         */
template <class T> inline vector3t<T> normalize(const vector3t<T> &rhs) {
  return vector3t<T>(rhs).normalize();
}

/**
         *	@param lhs a vector.
         *	@param rhs a vector.
         *	@return lhs &dot; rhs
         */
template <class T>
inline T dot(const vector3t<T> &lhs, const vector3t<T> &rhs) {
  return (lhs[0] * rhs[0]) + (lhs[1] * rhs[1]) + (lhs[2] * rhs[2]);
}

/**
         *	@param lhs a vector.
         *	@param rhs a vector.
         *	@return lhs &cross; rhs
         */
template <class T>
inline vector3t<T> cross(const vector3t<T> &lhs, const vector3t<T> &rhs) {
  return vector3t<T>(lhs[1] * rhs[2] - lhs[2] * rhs[1], // xyzzy
                     lhs[2] * rhs[0] - lhs[0] * rhs[2], // yzxxz
                     lhs[0] * rhs[1] - lhs[1] * rhs[0] // zxyyx
                     );
}

//@}

//--------------------------------------------------
// compare
/**
         *	@name comparer
         *	@relates vector
         */
//@{

/**
         *	@param lhs a vector.
         *	@param rhs a vector.
         *	@return lhs == rhs
         */
template <class T>
inline bool operator==(const vector3t<T> &lhs, const vector3t<T> &rhs) {
  return (lhs[0] == rhs[0]) && (lhs[1] == rhs[1]) && (lhs[2] == rhs[2]);
}

/**
         *	@param lhs a vector.
         *	@param rhs a vector.
         *	@return lhs != rhs
         */
template <class T>
inline bool operator!=(const vector3t<T> &lhs, const vector3t<T> &rhs) {
  return !(lhs == rhs);
}

//@}

//-----------------------------------------------
// output
/**
         *	@name output
         *	@relates vector3
         */
//@{

/**
         *	ostream <<
         */
template <class T, class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os, const vector3t<T> &rhs) {
  std::basic_ostringstream<CharT, Traits> s;
  s.flags(os.flags());
  s.imbue(os.getloc());
  s.precision(os.precision());
  s << "(";
  for (std::size_t i = 0; i < 2; ++i) {
    s << rhs[i] << ",";
  }
  s << rhs[2] << ")";
  return os << s.str();
}
//@}

typedef vector3t<float> vector3;
typedef vector3t<float> vector3f;
typedef vector3t<double> vector3d;
}

#endif
