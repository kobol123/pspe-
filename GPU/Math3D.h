// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <cmath>
#include "Common/Common.h"

#include "math/fast/fast_matrix.h"

#if defined(_M_SSE)
#include <emmintrin.h>
#endif

namespace Math3D {

// Helper for Vec classes to clamp values.
template<typename T>
inline static T VecClamp(const T &v, const T &low, const T &high)
{
	if (v > high)
		return high;
	if (v < low)
		return low;
	return v;
}

template<typename T>
class Vec2
{
public:
	union
	{
		struct
		{
			T x,y;
		};
#if defined(_M_SSE)
		__m128i ivec;
		__m128 vec;
#endif
	};

	T* AsArray() { return &x; }
	const T* AsArray() const { return &x; }

	Vec2() {}
	Vec2(const T a[2]) : x(a[0]), y(a[1]) {}
	Vec2(const T& _x, const T& _y) : x(_x), y(_y) {}
#if defined(_M_SSE)
	Vec2(const __m128 &_vec) : vec(_vec) {}
	Vec2(const __m128i &_ivec) : ivec(_ivec) {}
#endif

	template<typename T2>
	Vec2<T2> Cast() const
	{
		return Vec2<T2>((T2)x, (T2)y);
	}

	static Vec2 AssignToAll(const T& f)
	{
		return Vec2<T>(f, f);
	}

	void Write(T a[2])
	{
		a[0] = x; a[1] = y;
	}

	Vec2 operator +(const Vec2& other) const
	{
		return Vec2(x+other.x, y+other.y);
	}
	void operator += (const Vec2 &other)
	{
		x+=other.x; y+=other.y;
	}
	Vec2 operator -(const Vec2& other) const
	{
		return Vec2(x-other.x, y-other.y);
	}
	void operator -= (const Vec2& other)
	{
		x-=other.x; y-=other.y;
	}
	Vec2 operator -() const
	{
		return Vec2(-x,-y);
	}
	Vec2 operator * (const Vec2& other) const
	{
		return Vec2(x*other.x, y*other.y);
	}
	template<typename V>
	Vec2 operator * (const V& f) const
	{
		return Vec2(x*f,y*f);
	}
	template<typename V>
	void operator *= (const V& f)
	{
		x*=f; y*=f;
	}
	template<typename V>
	Vec2 operator / (const V& f) const
	{
		return Vec2(x/f,y/f);
	}
	template<typename V>
	void operator /= (const V& f)
	{
		*this = *this / f;
	}

	T Length2() const
	{
		return x*x + y*y;
	}

	Vec2 Clamp(const T &l, const T &h) const
	{
		return Vec2(VecClamp(x, l, h), VecClamp(y, l, h));
	}

	// Only implemented for T=float
	float Length() const;
	void SetLength(const float l);
	Vec2 WithLength(const float l) const;
	float Distance2To(Vec2 &other);
	Vec2 Normalized() const;
	float Normalize(); // returns the previous length, which is often useful

	T& operator [] (int i) //allow vector[1] = 3   (vector.y=3)
	{
		return *((&x) + i);
	}
	T operator [] (const int i) const
	{
		return *((&x) + i);
	}

	void SetZero()
	{
		x=0; y=0;
	}

	// Common aliases: UV (texel coordinates), ST (texture coordinates)
	T& u() { return x; }
	T& v() { return y; }
	T& s() { return x; }
	T& t() { return y; }

	const T& u() const { return x; }
	const T& v() const { return y; }
	const T& s() const { return x; }
	const T& t() const { return y; }

	// swizzlers - create a subvector of specific components
	const Vec2 yx() const { return Vec2(y, x); }
	const Vec2 vu() const { return Vec2(y, x); }
	const Vec2 ts() const { return Vec2(y, x); }
};

typedef Vec2<float> Vec2f;

template<typename T>
class Vec3
{
public:
	union
	{
		struct
		{
			T x,y,z;
		};
#if defined(_M_SSE)
		__m128i ivec;
		__m128 vec;
#endif
	};

	T* AsArray() { return &x; }
	const T* AsArray() const { return &x; }

	Vec3() {}
	Vec3(const T a[3]) : x(a[0]), y(a[1]), z(a[2]) {}
	Vec3(const T& _x, const T& _y, const T& _z) : x(_x), y(_y), z(_z) {}
	Vec3(const Vec2<T>& _xy, const T& _z) : x(_xy.x), y(_xy.y), z(_z) {}
#if defined(_M_SSE)
	Vec3(const __m128 &_vec) : vec(_vec) {}
	Vec3(const __m128i &_ivec) : ivec(_ivec) {}
#endif

	template<typename T2>
	Vec3<T2> Cast() const
	{
		return Vec3<T2>((T2)x, (T2)y, (T2)z);
	}

	// Only implemented for T=int and T=float
	static Vec3 FromRGB(unsigned int rgb);
	unsigned int ToRGB() const; // alpha bits set to zero

	static Vec3 AssignToAll(const T& f)
	{
		return Vec3<T>(f, f, f);
	}

	void Write(T a[3])
	{
		a[0] = x; a[1] = y; a[2] = z;
	}

	Vec3 operator +(const Vec3 &other) const
	{
		return Vec3(x+other.x, y+other.y, z+other.z);
	}
	void operator += (const Vec3 &other)
	{
		x+=other.x; y+=other.y; z+=other.z;
	}
	Vec3 operator -(const Vec3 &other) const
	{
		return Vec3(x-other.x, y-other.y, z-other.z);
	}
	void operator -= (const Vec3 &other)
	{
		x-=other.x; y-=other.y; z-=other.z;
	}
	Vec3 operator -() const
	{
		return Vec3(-x,-y,-z);
	}
	Vec3 operator * (const Vec3 &other) const
	{
		return Vec3(x*other.x, y*other.y, z*other.z);
	}
	template<typename V>
	Vec3 operator * (const V& f) const
	{
		return Vec3(x*f,y*f,z*f);
	}
	template<typename V>
	void operator *= (const V& f)
	{
		x*=f; y*=f; z*=f;
	}
	template<typename V>
	Vec3 operator / (const V& f) const
	{
		return Vec3(x/f,y/f,z/f);
	}
	template<typename V>
	void operator /= (const V& f)
	{
		*this = *this / f;
	}

	T Length2() const
	{
		return x*x + y*y + z*z;
	}

	Vec3 Clamp(const T &l, const T &h) const
	{
		return Vec3(VecClamp(x, l, h), VecClamp(y, l, h), VecClamp(z, l, h));
	}

	// Only implemented for T=float
	float Length() const;
	void SetLength(const float l);
	Vec3 WithLength(const float l) const;
	float Distance2To(Vec3 &other);
	Vec3 Normalized() const;
	float Normalize(); // returns the previous length, which is often useful

	T& operator [] (int i) //allow vector[2] = 3   (vector.z=3)
	{
		return *((&x) + i);
	}
	T operator [] (const int i) const
	{
		return *((&x) + i);
	}

	void SetZero()
	{
		x=0; y=0; z=0;
	}

	// Common aliases: UVW (texel coordinates), RGB (colors), STQ (texture coordinates)
	T& u() { return x; }
	T& v() { return y; }
	T& w() { return z; }

	T& r() { return x; }
	T& g() { return y; }
	T& b() { return z; }

	T& s() { return x; }
	T& t() { return y; }
	T& q() { return z; }

	const T& u() const { return x; }
	const T& v() const { return y; }
	const T& w() const { return z; }

	const T& r() const { return x; }
	const T& g() const { return y; }
	const T& b() const { return z; }

	const T& s() const { return x; }
	const T& t() const { return y; }
	const T& q() const { return z; }

	// swizzlers - create a subvector of specific components
	// e.g. Vec2 uv() { return Vec2(x,y); }
	// _DEFINE_SWIZZLER2 defines a single such function, DEFINE_SWIZZLER2 defines all of them for all component names (x<->r) and permutations (xy<->yx)
#define _DEFINE_SWIZZLER2(a, b, name) const Vec2<T> name() const { return Vec2<T>(a, b); }
#define DEFINE_SWIZZLER2(a, b, a2, b2, a3, b3, a4, b4) \
	_DEFINE_SWIZZLER2(a, b, a##b); \
	_DEFINE_SWIZZLER2(a, b, a2##b2); \
	_DEFINE_SWIZZLER2(a, b, a3##b3); \
	_DEFINE_SWIZZLER2(a, b, a4##b4); \
	_DEFINE_SWIZZLER2(b, a, b##a); \
	_DEFINE_SWIZZLER2(b, a, b2##a2); \
	_DEFINE_SWIZZLER2(b, a, b3##a3); \
	_DEFINE_SWIZZLER2(b, a, b4##a4);

	DEFINE_SWIZZLER2(x, y, r, g, u, v, s, t);
	DEFINE_SWIZZLER2(x, z, r, b, u, w, s, q);
	DEFINE_SWIZZLER2(y, z, g, b, v, w, t, q);
#undef DEFINE_SWIZZLER2
#undef _DEFINE_SWIZZLER2
};

template<typename T>
class Vec3Packed
{
public:
	union
	{
		struct
		{
			T x,y,z;
		};
	};

	T* AsArray() { return &x; }
	const T* AsArray() const { return &x; }

	Vec3Packed() {}
	Vec3Packed(const T a[3]) : x(a[0]), y(a[1]), z(a[2]) {}
	Vec3Packed(const T& _x, const T& _y, const T& _z) : x(_x), y(_y), z(_z) {}
	Vec3Packed(const Vec2<T>& _xy, const T& _z) : x(_xy.x), y(_xy.y), z(_z) {}

	template<typename T2>
	Vec3Packed<T2> Cast() const
	{
		return Vec3Packed<T2>((T2)x, (T2)y, (T2)z);
	}

	// Only implemented for T=int and T=float
	static Vec3Packed FromRGB(unsigned int rgb);
	unsigned int ToRGB() const; // alpha bits set to zero

	static Vec3Packed AssignToAll(const T& f)
	{
		return Vec3Packed<T>(f, f, f);
	}

	void Write(T a[3])
	{
		a[0] = x; a[1] = y; a[2] = z;
	}

	Vec3Packed operator +(const Vec3Packed &other) const
	{
		return Vec3Packed(x+other.x, y+other.y, z+other.z);
	}
	void operator += (const Vec3Packed &other)
	{
		x+=other.x; y+=other.y; z+=other.z;
	}
	Vec3Packed operator -(const Vec3Packed &other) const
	{
		return Vec3Packed(x-other.x, y-other.y, z-other.z);
	}
	void operator -= (const Vec3Packed &other)
	{
		x-=other.x; y-=other.y; z-=other.z;
	}
	Vec3Packed operator -() const
	{
		return Vec3Packed(-x,-y,-z);
	}
	Vec3Packed operator * (const Vec3Packed &other) const
	{
		return Vec3Packed(x*other.x, y*other.y, z*other.z);
	}
	template<typename V>
	Vec3Packed operator * (const V& f) const
	{
		return Vec3Packed(x*f,y*f,z*f);
	}
	template<typename V>
	void operator *= (const V& f)
	{
		x*=f; y*=f; z*=f;
	}
	template<typename V>
	Vec3Packed operator / (const V& f) const
	{
		return Vec3Packed(x/f,y/f,z/f);
	}
	template<typename V>
	void operator /= (const V& f)
	{
		*this = *this / f;
	}

	T Length2() const
	{
		return x*x + y*y + z*z;
	}

	Vec3Packed Clamp(const T &l, const T &h) const
	{
		return Vec3Packed(VecClamp(x, l, h), VecClamp(y, l, h), VecClamp(z, l, h));
	}

	// Only implemented for T=float
	float Length() const;
	void SetLength(const float l);
	Vec3Packed WithLength(const float l) const;
	float Distance2To(Vec3Packed &other);
	Vec3Packed Normalized() const;
	float Normalize(); // returns the previous length, which is often useful

	T& operator [] (int i) //allow vector[2] = 3   (vector.z=3)
	{
		return *((&x) + i);
	}
	T operator [] (const int i) const
	{
		return *((&x) + i);
	}

	void SetZero()
	{
		x=0; y=0; z=0;
	}

	// Common aliases: UVW (texel coordinates), RGB (colors), STQ (texture coordinates)
	T& u() { return x; }
	T& v() { return y; }
	T& w() { return z; }

	T& r() { return x; }
	T& g() { return y; }
	T& b() { return z; }

	T& s() { return x; }
	T& t() { return y; }
	T& q() { return z; }

	const T& u() const { return x; }
	const T& v() const { return y; }
	const T& w() const { return z; }

	const T& r() const { return x; }
	const T& g() const { return y; }
	const T& b() const { return z; }

	const T& s() const { return x; }
	const T& t() const { return y; }
	const T& q() const { return z; }

	// swizzlers - create a subvector of specific components
	// e.g. Vec2 uv() { return Vec2(x,y); }
	// _DEFINE_SWIZZLER2 defines a single such function, DEFINE_SWIZZLER2 defines all of them for all component names (x<->r) and permutations (xy<->yx)
#define _DEFINE_SWIZZLER2(a, b, name) const Vec2<T> name() const { return Vec2<T>(a, b); }
#define DEFINE_SWIZZLER2(a, b, a2, b2, a3, b3, a4, b4) \
	_DEFINE_SWIZZLER2(a, b, a##b); \
	_DEFINE_SWIZZLER2(a, b, a2##b2); \
	_DEFINE_SWIZZLER2(a, b, a3##b3); \
	_DEFINE_SWIZZLER2(a, b, a4##b4); \
	_DEFINE_SWIZZLER2(b, a, b##a); \
	_DEFINE_SWIZZLER2(b, a, b2##a2); \
	_DEFINE_SWIZZLER2(b, a, b3##a3); \
	_DEFINE_SWIZZLER2(b, a, b4##a4);

	DEFINE_SWIZZLER2(x, y, r, g, u, v, s, t);
	DEFINE_SWIZZLER2(x, z, r, b, u, w, s, q);
	DEFINE_SWIZZLER2(y, z, g, b, v, w, t, q);
#undef DEFINE_SWIZZLER2
#undef _DEFINE_SWIZZLER2
};

template<typename T>
class Vec4
{
public:
	union
	{
		struct
		{
			T x,y,z,w;
		};
#if defined(_M_SSE)
		__m128i ivec;
		__m128 vec;
#endif
	};

	T* AsArray() { return &x; }
	const T* AsArray() const { return &x; }

	Vec4() {}
	Vec4(const T a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3]) {}
	Vec4(const T& _x, const T& _y, const T& _z, const T& _w) : x(_x), y(_y), z(_z), w(_w) {}
	Vec4(const Vec2<T>& _xy, const T& _z, const T& _w) : x(_xy.x), y(_xy.y), z(_z), w(_w) {}
	Vec4(const Vec3<T>& _xyz, const T& _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}
#if defined(_M_SSE)
	Vec4(const __m128 &_vec) : vec(_vec) {}
	Vec4(const __m128i &_ivec) : ivec(_ivec) {}
#endif

	template<typename T2>
	Vec4<T2> Cast() const
	{
		return Vec4<T2>((T2)x, (T2)y, (T2)z, (T2)w);
	}

	// Only implemented for T=int and T=float
	static Vec4 FromRGBA(unsigned int rgba);
	unsigned int ToRGBA() const;

	static Vec4 AssignToAll(const T& f)
	{
		return Vec4<T>(f, f, f, f);
	}

	void Write(T a[4])
	{
		a[0] = x; a[1] = y; a[2] = z; a[3] = w;
	}

	Vec4 operator +(const Vec4& other) const
	{
		return Vec4(x+other.x, y+other.y, z+other.z, w+other.w);
	}
	void operator += (const Vec4& other)
	{
		x+=other.x; y+=other.y; z+=other.z; w+=other.w;
	}
	Vec4 operator -(const Vec4 &other) const
	{
		return Vec4(x-other.x, y-other.y, z-other.z, w-other.w);
	}
	void operator -= (const Vec4 &other)
	{
		x-=other.x; y-=other.y; z-=other.z; w-=other.w;
	}
	Vec4 operator -() const
	{
		return Vec4(-x,-y,-z,-w);
	}
	Vec4 operator * (const Vec4 &other) const
	{
		return Vec4(x*other.x, y*other.y, z*other.z, w*other.w);
	}
	template<typename V>
	Vec4 operator * (const V& f) const
	{
		return Vec4(x*f,y*f,z*f,w*f);
	}
	template<typename V>
	void operator *= (const V& f)
	{
		x*=f; y*=f; z*=f; w*=f;
	}
	template<typename V>
	Vec4 operator / (const V& f) const
	{
		return Vec4(x/f,y/f,z/f,w/f);
	}
	template<typename V>
	void operator /= (const V& f)
	{
		*this = *this / f;
	}

	T Length2() const
	{
		return x*x + y*y + z*z + w*w;
	}

	Vec4 Clamp(const T &l, const T &h) const
	{
		return Vec4(VecClamp(x, l, h), VecClamp(y, l, h), VecClamp(z, l, h), VecClamp(w, l, h));
	}

	// Only implemented for T=float
	float Length() const;
	void SetLength(const float l);
	Vec4 WithLength(const float l) const;
	float Distance2To(Vec4 &other);
	Vec4 Normalized() const;
	float Normalize(); // returns the previous length, which is often useful

	T& operator [] (int i) //allow vector[2] = 3   (vector.z=3)
	{
		return *((&x) + i);
	}
	T operator [] (const int i) const
	{
		return *((&x) + i);
	}

	void SetZero()
	{
		x=0; y=0; z=0;
	}

	// Common alias: RGBA (colors)
	T& r() { return x; }
	T& g() { return y; }
	T& b() { return z; }
	T& a() { return w; }

	const T& r() const { return x; }
	const T& g() const { return y; }
	const T& b() const { return z; }
	const T& a() const { return w; }

	// swizzlers - create a subvector of specific components
	// e.g. Vec2 uv() { return Vec2(x,y); }
	// _DEFINE_SWIZZLER2 defines a single such function, DEFINE_SWIZZLER2 defines all of them for all component names (x<->r) and permutations (xy<->yx)
#define _DEFINE_SWIZZLER2(a, b, name) const Vec2<T> name() const { return Vec2<T>(a, b); }
#define DEFINE_SWIZZLER2(a, b, a2, b2) \
	_DEFINE_SWIZZLER2(a, b, a##b); \
	_DEFINE_SWIZZLER2(a, b, a2##b2); \
	_DEFINE_SWIZZLER2(b, a, b##a); \
	_DEFINE_SWIZZLER2(b, a, b2##a2);

	DEFINE_SWIZZLER2(x, y, r, g);
	DEFINE_SWIZZLER2(x, z, r, b);
	DEFINE_SWIZZLER2(x, w, r, a);
	DEFINE_SWIZZLER2(y, z, g, b);
	DEFINE_SWIZZLER2(y, w, g, a);
	DEFINE_SWIZZLER2(z, w, b, a);
#undef DEFINE_SWIZZLER2
#undef _DEFINE_SWIZZLER2

#define _DEFINE_SWIZZLER3(a, b, c, name) const Vec3<T> name() const { return Vec3<T>(a, b, c); }
#define DEFINE_SWIZZLER3(a, b, c, a2, b2, c2) \
	_DEFINE_SWIZZLER3(a, b, c, a##b##c); \
	_DEFINE_SWIZZLER3(a, c, b, a##c##b); \
	_DEFINE_SWIZZLER3(b, a, c, b##a##c); \
	_DEFINE_SWIZZLER3(b, c, a, b##c##a); \
	_DEFINE_SWIZZLER3(c, a, b, c##a##b); \
	_DEFINE_SWIZZLER3(c, b, a, c##b##a); \
	_DEFINE_SWIZZLER3(a, b, c, a2##b2##c2); \
	_DEFINE_SWIZZLER3(a, c, b, a2##c2##b2); \
	_DEFINE_SWIZZLER3(b, a, c, b2##a2##c2); \
	_DEFINE_SWIZZLER3(b, c, a, b2##c2##a2); \
	_DEFINE_SWIZZLER3(c, a, b, c2##a2##b2); \
	_DEFINE_SWIZZLER3(c, b, a, c2##b2##a2);

	DEFINE_SWIZZLER3(x, y, z, r, g, b);
	DEFINE_SWIZZLER3(x, y, w, r, g, a);
	DEFINE_SWIZZLER3(x, z, w, r, b, a);
	DEFINE_SWIZZLER3(y, z, w, g, b, a);
#undef DEFINE_SWIZZLER3
#undef _DEFINE_SWIZZLER3
};


template<typename BaseType>
class Mat3x3
{
public:
	// Convention: first three values = first column
	Mat3x3(const BaseType values[])
	{
		for (unsigned int i = 0; i < 3*3; ++i)
		{
			this->values[i] = values[i];
		}
	}

	Mat3x3(BaseType _00, BaseType _01, BaseType _02, BaseType _10, BaseType _11, BaseType _12, BaseType _20, BaseType _21, BaseType _22)
	{
		values[0] = _00;
		values[1] = _01;
		values[2] = _02;
		values[3] = _10;
		values[4] = _11;
		values[5] = _12;
		values[6] = _20;
		values[7] = _21;
		values[8] = _22;
	}

	template<typename T>
	Vec3<T> operator * (const Vec3<T>& vec) const
	{
		Vec3<T> ret;
		ret.x = values[0]*vec.x + values[3]*vec.y + values[6]*vec.z;
		ret.y = values[1]*vec.x + values[4]*vec.y + values[7]*vec.z;
		ret.z = values[2]*vec.x + values[5]*vec.y + values[8]*vec.z;
		return ret;
	}

	Mat3x3 Inverse() const
	{
		float a = values[0];
		float b = values[1];
		float c = values[2];
		float d = values[3];
		float e = values[4];
		float f = values[5];
		float g = values[6];
		float h = values[7];
		float i = values[8];
		return Mat3x3(e*i-f*h, f*g-d*i, d*h-e*g,
						c*h-b*i, a*i-c*g, b*g-a*h,
						b*f-c*e, c*d-a*f, a*e-b*d) / Det();
	}

	BaseType Det() const
	{
		return values[0]*values[4]*values[8] + values[3]*values[7]*values[2] +
				values[6]*values[1]*values[5] - values[2]*values[4]*values[6] -
				values[5]*values[7]*values[0] - values[8]*values[1]*values[3];
	}

	Mat3x3 operator / (const BaseType& val) const
	{
		return Mat3x3(values[0]/val, values[1]/val, values[2]/val,
						values[3]/val, values[4]/val, values[5]/val,
						values[6]/val, values[7]/val, values[8]/val);
	}

private:
	BaseType values[3*3];
};


template<typename BaseType>
class Mat4x4
{
public:
	// Convention: first four values in arrow = first column
	Mat4x4(const BaseType values[])
	{
		for (unsigned int i = 0; i < 4*4; ++i)
		{
			this->values[i] = values[i];
		}
	}

	template<typename T>
	Vec4<T> operator * (const Vec4<T>& vec) const
	{
		Vec4<T> ret;
		ret.x = values[0]*vec.x + values[4]*vec.y + values[8]*vec.z + values[12]*vec.w;
		ret.y = values[1]*vec.x + values[5]*vec.y + values[9]*vec.z + values[13]*vec.w;
		ret.z = values[2]*vec.x + values[6]*vec.y + values[10]*vec.z + values[14]*vec.w;
		ret.w = values[3]*vec.x + values[7]*vec.y + values[11]*vec.z + values[15]*vec.w;
		return ret;
	}

private:
	BaseType values[4*4];
};

}; // namespace Math3D

typedef Math3D::Vec3<float> Vec3f;
typedef Math3D::Vec3Packed<float> Vec3Packedf;
typedef Math3D::Vec4<float> Vec4f;


inline void Vec3ByMatrix43(float vecOut[3], const float v[3], const float m[12])
{
	vecOut[0] = v[0] * m[0] + v[1] * m[3] + v[2] * m[6] + m[9];
	vecOut[1] = v[0] * m[1] + v[1] * m[4] + v[2] * m[7] + m[10];
	vecOut[2] = v[0] * m[2] + v[1] * m[5] + v[2] * m[8] + m[11];
}

inline void Vec3ByMatrix44(float vecOut[4], const float v[3], const float m[16])
{
	vecOut[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + m[12];
	vecOut[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + m[13];
	vecOut[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + m[14];
	vecOut[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + m[15];
}

inline void Vec4ByMatrix44(float vecOut[4], const float v[4], const float m[16])
{
	vecOut[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
	vecOut[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
	vecOut[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
	vecOut[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
}


inline void Norm3ByMatrix43(float vecOut[3], const float v[3], const float m[12])
{
	vecOut[0] = v[0] * m[0] + v[1] * m[3] + v[2] * m[6];
	vecOut[1] = v[0] * m[1] + v[1] * m[4] + v[2] * m[7];
	vecOut[2] = v[0] * m[2] + v[1] * m[5] + v[2] * m[8];
}

inline void Matrix4ByMatrix4(float out[16], const float a[16], const float b[16]) {
	fast_matrix_mul_4x4(out, b, a);
}

inline void ConvertMatrix4x3To4x4(float *m4x4, const float *m4x3) {
	m4x4[0] = m4x3[0];
	m4x4[1] = m4x3[1];
	m4x4[2] = m4x3[2];
	m4x4[3] = 0.0f;
	m4x4[4] = m4x3[3];
	m4x4[5] = m4x3[4];
	m4x4[6] = m4x3[5];
	m4x4[7] = 0.0f;
	m4x4[8] = m4x3[6];
	m4x4[9] = m4x3[7];
	m4x4[10] = m4x3[8];
	m4x4[11] = 0.0f;
	m4x4[12] = m4x3[9];
	m4x4[13] = m4x3[10];
	m4x4[14] = m4x3[11];
	m4x4[15] = 1.0f;
}

inline void ConvertMatrix4x3To4x4Transposed(float *m4x4, const float *m4x3) {
	m4x4[0] = m4x3[0];
	m4x4[1] = m4x3[3];
	m4x4[2] = m4x3[6];
	m4x4[3] = m4x3[9];
	m4x4[4] = m4x3[1];
	m4x4[5] = m4x3[4];
	m4x4[6] = m4x3[7];
	m4x4[7] = m4x3[10];
	m4x4[8] = m4x3[2];
	m4x4[9] = m4x3[5];
	m4x4[10] = m4x3[8];
	m4x4[11] = m4x3[11];
	m4x4[12] = 0.0f;
	m4x4[13] = 0.0f;
	m4x4[14] = 0.0f;
	m4x4[15] = 1.0f;
}

inline void ConvertMatrix4x3To3x4Transposed(float *m4x4, const float *m4x3) {
	m4x4[0] = m4x3[0];
	m4x4[1] = m4x3[3];
	m4x4[2] = m4x3[6];
	m4x4[3] = m4x3[9];
	m4x4[4] = m4x3[1];
	m4x4[5] = m4x3[4];
	m4x4[6] = m4x3[7];
	m4x4[7] = m4x3[10];
	m4x4[8] = m4x3[2];
	m4x4[9] = m4x3[5];
	m4x4[10] = m4x3[8];
	m4x4[11] = m4x3[11];
}

inline void Transpose4x4(float out[16], const float in[16]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[i * 4 + j] = in[j * 4 + i];
		}
	}
}

inline float Vec3Dot(const float v1[3], const float v2[3])
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

namespace Math3D {

template<typename T>
inline T Dot(const Vec2<T>& a, const Vec2<T>& b)
{
	return a.x*b.x + a.y*b.y;
}

template<typename T>
inline T Dot(const Vec3<T>& a, const Vec3<T>& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

template<typename T>
inline T Dot(const Vec4<T>& a, const Vec4<T>& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

template<typename T>
inline Vec3<T> Cross(const Vec3<T>& a, const Vec3<T>& b)
{
	return Vec3<T>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

template<typename T>
inline Vec3Packed<T> Cross(const Vec3Packed<T>& a, const Vec3Packed<T>& b)
{
	return Vec3Packed<T>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

template<>
inline Vec3<float> Vec3<float>::FromRGB(unsigned int rgb)
{
#if defined(_M_SSE)
	__m128i z = _mm_setzero_si128();
	__m128i c = _mm_cvtsi32_si128(rgb);
	c = _mm_unpacklo_epi16(_mm_unpacklo_epi8(c, z), z);
	return Vec3<float>(_mm_mul_ps(_mm_cvtepi32_ps(c), _mm_set_ps1(1.0f / 255.0f)));
#else
	return Vec3((rgb & 0xFF) * (1.0f/255.0f),
				((rgb >> 8) & 0xFF) * (1.0f/255.0f),
				((rgb >> 16) & 0xFF) * (1.0f/255.0f));
#endif
}

template<>
inline Vec3<int> Vec3<int>::FromRGB(unsigned int rgb)
{
#if defined(_M_SSE)
	__m128i z = _mm_setzero_si128();
	__m128i c = _mm_cvtsi32_si128(rgb);
	c = _mm_unpacklo_epi16(_mm_unpacklo_epi8(c, z), z);
	return Vec3<int>(c);
#else
	return Vec3(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
#endif
}

template<>
__forceinline unsigned int Vec3<float>::ToRGB() const
{
#if defined(_M_SSE)
	__m128i c = _mm_cvtps_epi32(_mm_mul_ps(vec, _mm_set_ps1(255.0f)));
	__m128i c16 = _mm_packs_epi32(c, c);
	return _mm_cvtsi128_si32(_mm_packus_epi16(c16, c16)) & 0x00FFFFFF;
#else
	return ((unsigned int)(r()*255.f) << 0) |
			((unsigned int)(g()*255.f) << 8) |
			((unsigned int)(b()*255.f) << 16);
#endif
}

template<>
__forceinline unsigned int Vec3<int>::ToRGB() const
{
#if defined(_M_SSE)
	__m128i c16 = _mm_packs_epi32(ivec, ivec);
	return _mm_cvtsi128_si32(_mm_packus_epi16(c16, c16)) & 0x00FFFFFF;
#else
	return (r()&0xFF) | ((g()&0xFF)<<8) | ((b()&0xFF)<<16);
#endif
}

template<>
inline Vec4<float> Vec4<float>::FromRGBA(unsigned int rgba)
{
#if defined(_M_SSE)
	__m128i z = _mm_setzero_si128();
	__m128i c = _mm_cvtsi32_si128(rgba);
	c = _mm_unpacklo_epi16(_mm_unpacklo_epi8(c, z), z);
	return Vec4<float>(_mm_mul_ps(_mm_cvtepi32_ps(c), _mm_set_ps1(1.0f / 255.0f)));
#else
	return Vec4((rgba & 0xFF) * (1.0f/255.0f),
				((rgba >> 8) & 0xFF) * (1.0f/255.0f),
				((rgba >> 16) & 0xFF) * (1.0f/255.0f),
				((rgba >> 24) & 0xFF) * (1.0f/255.0f));
#endif
}

template<>
inline Vec4<int> Vec4<int>::FromRGBA(unsigned int rgba)
{
#if defined(_M_SSE)
	__m128i z = _mm_setzero_si128();
	__m128i c = _mm_cvtsi32_si128(rgba);
	c = _mm_unpacklo_epi16(_mm_unpacklo_epi8(c, z), z);
	return Vec4<int>(c);
#else
	return Vec4(rgba & 0xFF, (rgba >> 8) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 24) & 0xFF);
#endif
}

template<>
__forceinline unsigned int Vec4<float>::ToRGBA() const
{
#if defined(_M_SSE)
	__m128i c = _mm_cvtps_epi32(_mm_mul_ps(vec, _mm_set_ps1(255.0f)));
	__m128i c16 = _mm_packs_epi32(c, c);
	return _mm_cvtsi128_si32(_mm_packus_epi16(c16, c16));
#else
	return ((unsigned int)(r()*255.f) << 0) |
			((unsigned int)(g()*255.f) << 8) |
			((unsigned int)(b()*255.f) << 16) |
			((unsigned int)(a()*255.f) << 24);
#endif
}

template<>
__forceinline unsigned int Vec4<int>::ToRGBA() const
{
#if defined(_M_SSE)
	__m128i c16 = _mm_packs_epi32(ivec, ivec);
	return _mm_cvtsi128_si32(_mm_packus_epi16(c16, c16));
#else
	return (r()&0xFF) | ((g()&0xFF)<<8) | ((b()&0xFF)<<16) | ((a()&0xFF)<<24);
#endif
}

}; // namespace Math3D

// linear interpolation via float: 0.0=begin, 1.0=end
template<typename X>
inline X Lerp(const X& begin, const X& end, const float t)
{
	return begin*(1.f-t) + end*t;
}

// linear interpolation via int: 0=begin, base=end
template<typename X, int base>
inline X LerpInt(const X& begin, const X& end, const int t)
{
	return (begin*(base-t) + end*t) / base;
}
