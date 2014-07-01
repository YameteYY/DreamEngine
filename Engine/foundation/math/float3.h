/****************************************************************************
Copyright (c) 2007,RadonLabs GmbH
Copyright (c) 2011-2013,WebJet Business Division,CYOU
 
http://www.genesis-3d.com.cn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#ifndef __float3_H__
#define __float3_H__



#include "math/matrix44.h"
#include "math/float4.h"

namespace Math
{
class float3
{
public:
	//static int testFlag;
	/// default constructor, NOTE: does NOT setup components!
	float3();
	/// construct from values
	float3(float x, float y, float z);
	explicit float3(const float* v);
	explicit float3(float s);
	/// copy constructor
	float3(const float3& rhs);
	/// assignment operator
	float3& operator = (const float3& rhs);
	float3& operator = (float s);
	/// Pointer accessor for direct copying
	float* ptr();
	/// Pointer accessor for direct copying
	const float* ptr() const;

	float operator [] ( const int i ) const;
	float& operator [] ( const int i );

	/// equality operator
	bool operator == (const float3& rhs) const;
	/// inequality operator
	bool operator != (const float3& rhs) const;
	/// Returns true if the float3's float components are all greater that the ones of the float3 it is compared against
	bool operator < ( const float3& rhs ) const;
	/// Returns true if the vector's float components are all smaller that the ones of the vector it is compared against
	bool operator > ( const float3& rhs ) const;

	/// operator +
	const float3& operator + () const;
	/// flip sign
	float3 operator - () const;

	// arithmetic operations
	float3 operator + (const float3& rhs) const;
	float3 operator - (const float3& rhs) const;
	float3 operator * (float ffloat) const;
	float3 operator * (const float3& rhs) const;
	float3 operator / (float ffloat) const;
	float3 operator / ( const float3& rhs) const;

	// overloaded operators to help float3
	friend float3 operator * (float ffloat, const float3& rhs);
	friend float3 operator / (float ffloat, const float3& rhs);
	friend float3 operator + (const float3& lhs, float rhs);
	friend float3 operator + (float lhs, const float3& rhs);
	friend float3 operator - (const float3& lhs, float rhs);
	friend float3 operator - (float lhs, const float3& rhs);

	// arithmetic updates
	float3& operator+=(const float3& rhs);
	float3& operator+=(float s);
	float3& operator-=(const float3& rhs);
	float3& operator-=(float s);
	float3& operator*=(const float3& rhs);
	float3& operator*=(float s);
	float3& operator/=(const float3& rhs);
	float3& operator/=(float ffloat);

	/// set content
	void set(float x, float y, float z);
	/// set content
	void setFromFloat4(const float4& f4);
	/// read/write access to x component
	float& x();
	/// read/write access to y component
	float& y();
	/// read/write access to z component
	float& z();
	/// read-only access to x component
	float x() const;
	/// read-only access to y component
	float y() const;
	/// read-only access to z component
	float z() const;

	/// return length of vector
	float length() const;
	/// return squared length of vector
	float lengthsq() const;
	/// return the distance to another vector
	float distance(const float3& rhs) const;
	/// return the distance to another vector
	float distance(const plane& rhs) const;
	/// return the squared distance to another vector
	float distancesq(const float3& rhs) const;
	/// Calculates the dot (float) product of this float3 with another.
	float dotProduct(const float3& vec) const;
	/// Calculates the absolute dot (float) product of this float3 with another.
	float absDotProduct(const float3& vec) const;
	/// Calculates the cross-product of 2 float3
	float3 crossProduct( const float3& rkVector ) const;
	/// Normalises the vector.
	float normalise();
	/// Returns a float3 at a point half way between this and the passed in float3
	float3 midPoint( const float3& vec ) const;
	/// return compononent-wise absolute
	float3 abs() const;
	/// return true if any components are non-zero
	bool any() const;
	/// return true if all components are non-zero
	bool all() const;
	/// Check whether this vector contains valid values
	bool isNaN() const;
	/// transform float3 as point
	float3 transformPoint(const Math::matrix44& m) const;
	/// tansform float3 as vector
	float3 transformVector(const Math::matrix44& m) const;
	/// Generates a vector perpendicular to this vector (eg an 'up' vector).
	float3 perpendicular() const;

	/// return vector made up of largest components of 2 vectors
	static float3 maximize(const float3& v0, const float3& v1);
	/// return vector made up of smallest components of 2 vectors
	static float3 minimize(const float3& v0, const float3& v1);
	/// return normalized version of vector
	static float3 normalize(const float3& v);
	/// return if is normalized
	static bool isNormalize(const float3& v);
	/// set less-then components to non-zero 
	static float3 lt(const float3& v0, const float3& v1);
	/// set less-or-equal components to non-zero
	static float3 le(const float3& v0, const float3& v1);
	/// set greater-then components to non-zero
	static float3 gt(const float3& v0, const float3& v1);
	/// set greater-or-equal components to non-zero
	static float3 ge(const float3& v0, const float3& v1);
	/// compare two float3 with tolerance
	static bool compare(const float3& v0, const float3& v1, const float& tol);
	/// calculate a face normal without normalize, including the w component which is the offset from the origin.
	static float4 calculateFaceNormalWithoutNormalize(const float3& v1, const float3& v2, const float3& v3);
	/// Calculate a face normal without normalize, no w-information.
	static float3 calculateBasicFaceNormalWithoutNormalize(const float3& v1, const float3& v2, const float3& v3);
	///Hermite vector
	static void hermite(float3 *pOut,const float3 *pV1,const float3 *pT1,const float3 *pV2,const float3 *pT2,float s);
	/// convert to anything
	template<typename T> T as() const;
	
	static const float epsilon;

protected:
	float X;
	float Y;
	float Z;
};

//------------------------------------------------------------------------
inline 
float3::float3()
:X( 0.f ),Y(0.f),Z(0.f)
{
}
//------------------------------------------------------------------------
inline 
float3::float3(float x, float y, float z)
: X(x), Y(y), Z(z)
{
}
//------------------------------------------------------------------------
inline
float3::float3(const float* v)
: X(v[0]),Y(v[1]),Z(v[2])
{

}
//------------------------------------------------------------------------
inline 
float3::float3(float s)
: X(s), Y(s), Z(s)
{
}
//------------------------------------------------------------------------
inline 
float3::float3(const float3& rhs)
: X(rhs.X),Y(rhs.Y),Z(rhs.Z)
{
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator = (const float3& rhs)
{
	X = rhs.X;
	Y = rhs.Y;
	Z = rhs.Z;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator = (float s)
{
	X = s;
	Y = s;
	Z = s;
	return *this;
}
//------------------------------------------------------------------------
inline
float*
float3::ptr()
{
	return &X;
}
//------------------------------------------------------------------------
inline
const float*
float3::ptr() const
{
	return &X;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
inline
bool
float3::operator == (const float3& rhs) const
{
	return ( X == rhs.X && Y == rhs.Y && Z == rhs.Z );
}
//------------------------------------------------------------------------
inline 
bool 
float3::operator != (const float3& rhs) const
{
	return ( X != rhs.X || Y != rhs.Y || Z != rhs.Z );
}
//------------------------------------------------------------------------
inline 
bool 
float3::operator < ( const float3& rhs ) const
{
	return ( X < rhs.X && Y < rhs.Y && Z < rhs.Z );
}
//------------------------------------------------------------------------
inline 
bool 
float3::operator > ( const float3& rhs ) const
{
	return ( X > rhs.X && Y > rhs.Y && Z > rhs.Z );
}
//------------------------------------------------------------------------
inline 
const float3& 
float3::operator + () const
{
	return *this;
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator - () const
{
	return float3(-X, -Y, -Z);
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator + (const float3& rhs) const
{
	return float3(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator - (const float3& rhs) const
{
	return float3(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator * (float ffloat) const
{
	return float3(X * ffloat, Y * ffloat, Z * ffloat);
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator * (const float3& rhs) const
{
	return float3(X * rhs.X, Y * rhs.Y, Z * rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator / (float ffloat) const
{
	assert( ffloat != 0.0 );
	float fInv = 1.0f / ffloat;
	return float3(X * fInv, Y * fInv, Z * fInv);
}
//------------------------------------------------------------------------
inline 
float3 
float3::operator / ( const float3& rhs) const
{
	return float3( X / rhs.X, Y / rhs.Y, Z / rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3 
operator * (float ffloat, const float3& rhs)
{
	return float3(ffloat * rhs.X, ffloat * rhs.Y, ffloat * rhs.Z);
}
//------------------------------------------------------------------------
inline  
float3 
operator / (float ffloat, const float3& rhs)
{
	return float3(ffloat / rhs.X, ffloat / rhs.Y, ffloat / rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3 
operator + (const float3& lhs, float rhs)
{
	return float3(lhs.X + rhs, lhs.Y + rhs, lhs.Z + rhs);
}
//------------------------------------------------------------------------
inline 
float3 
operator + (float lhs, const float3& rhs)
{
	return float3(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3 
operator - (const float3& lhs, float rhs)
{
	return float3(lhs.X - rhs, lhs.Y - rhs, lhs.Z - rhs);
}
//------------------------------------------------------------------------
inline  
float3 
operator - (float lhs, const float3& rhs)
{
	return float3(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z);
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator+=(const float3& rhs)
{
	X += rhs.X;
	Y += rhs.Y;
	Z += rhs.Z;
	return *this;
}
//------------------------------------------------------------------------
inline float3& float3::operator+=(float s)
{
	X += s;
	Y += s;
	Z += s;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator-=(const float3& rhs)
{
	X -= rhs.X;
	Y -= rhs.Y;
	Z -= rhs.Z;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator-=(float s)
{
	X -= s;
	Y -= s;
	Z -= s;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3&
float3::operator*=(const float3& rhs)
{
	X *= rhs.X;
	Y *= rhs.Y;
	Z *= rhs.Z;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator*=(float s)
{
	X *= s;
	Y *= s;
	Z *= s;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator/=(const float3& rhs)
{
	X /= rhs.X;
	Y /= rhs.Y;
	Z /= rhs.Z;
	return *this;
}
//------------------------------------------------------------------------
inline 
float3& 
float3::operator/=(float ffloat)
{
	assert( ffloat != 0.0 );
	float fInv = 1.0f / ffloat;
	X *= fInv;
	Y *= fInv;
	Z *= fInv;
	return *this;
}
//------------------------------------------------------------------------
inline 
void 
float3::set(float x, float y, float z)
{
	X = x;
	Y = y;
	Z = z;
}

inline 
void 
float3::setFromFloat4(const float4& f4)
{
	X = f4.x();
	Y = f4.y();
	Z = f4.z();
}


//------------------------------------------------------------------------
inline 
float& 
float3::x()
{
	return X;
}
//------------------------------------------------------------------------
inline 
float& 
float3::y()
{
	return Y;
}
//------------------------------------------------------------------------
inline float& float3::z()
{
	return Z;
}
//------------------------------------------------------------------------
inline
float 
float3::x() const
{
	return X;
}
//------------------------------------------------------------------------
inline 
float 
float3::y() const
{
	return Y;
}
//------------------------------------------------------------------------
inline 
float 
float3::z() const
{
	return Z;
}
//------------------------------------------------------------------------
inline 
float 
float3::length() const
{
	return sqrt(X * X + Y * Y + Z * Z );
}
//------------------------------------------------------------------------
inline 
float 
float3::lengthsq() const
{
	return X * X + Y * Y + Z * Z ;
}
//------------------------------------------------------------------------
inline 
float 
float3::distance(const float3& rhs) const
{
	return (*this - rhs).length();
}
//------------------------------------------------------------------------
inline 
float 
float3::distance(const plane& rhs) const
{
	return X * rhs.a() + Y * rhs.b() + Z * rhs.c() + rhs.d();
}
//------------------------------------------------------------------------
inline 
float 
float3::distancesq(const float3& rhs) const
{
	return (*this - rhs).lengthsq();
}
//------------------------------------------------------------------------
inline
float 
float3::dotProduct(const float3& vec) const
{
	return X * vec.X + Y * vec.Y + Z * vec.Z;
}
//------------------------------------------------------------------------
inline
float 
float3::absDotProduct(const float3& vec) const
{
	return fabs(X * vec.X) + fabs(Y * vec.Y) + fabs(Z * vec.Z);
}
//------------------------------------------------------------------------
inline 
float3 
float3::crossProduct( const float3& rkVector ) const
{
	return float3(
		Y * rkVector.Z - Z * rkVector.Y,
		Z * rkVector.X - X * rkVector.Z,
		X * rkVector.Y - Y * rkVector.X);
}
//------------------------------------------------------------------------
inline 
float 
float3::normalise()
{
	float fLength = length();
	// Will also work for zero-sized vectors, but will change nothing
	if ( fLength > epsilon )
	{
		float fInvLength = 1.0f / fLength;
		X *= fInvLength;
		Y *= fInvLength;
		Z *= fInvLength;
	}

	return fLength;
}
//------------------------------------------------------------------------
inline
float3
float3::midPoint( const float3& vec ) const
{
	return float3(
		( X + vec.X ) * 0.5f,
		( Y + vec.Y ) * 0.5f,
		( Z + vec.Z ) * 0.5f );
}
//------------------------------------------------------------------------
inline
float3 
float3::abs() const
{
	return float3(fabs(X), fabs(Y) , fabs(Z));
}
//------------------------------------------------------------------------
inline
bool 
float3::any() const
{
	return (this->X != 0.0f) || (this->Y != 0.0f) || (this->Z != 0.0f );
}
//------------------------------------------------------------------------
inline
bool
float3::all() const
{
	return (this->X != 0.0f) && (this->Y != 0.0f) && (this->Z != 0.0f );
}
//------------------------------------------------------------------------
inline
bool
float3::isNaN() const
{
	return n_isNaN(X) || n_isNaN(Y) || n_isNaN(Z);
}
//------------------------------------------------------------------------
inline
float3 
float3::transformPoint(const Math::matrix44& m) const
{
	float4 p = float4::transform(m,float4(X,Y,Z,1));
	return float3(p.x(), p.y(),p.z());
}
//------------------------------------------------------------------------
inline
float3 
float3::transformVector(const Math::matrix44& m) const
{
	float4 p = float4::transform(m,float4(X,Y,Z,0));
	return float3(p.x(), p.y(),p.z());
}
//------------------------------------------------------------------------
inline 
float3 
float3::maximize(const float3& v0, const float3& v1)
{
	return float3(n_max(v0.X, v1.X), n_max(v0.Y, v1.Y),n_max(v0.Z, v1.Z));
}
//------------------------------------------------------------------------
inline 
float3 
float3::minimize(const float3& v0, const float3& v1)
{
	return float3(n_min(v0.X, v1.X), n_min(v0.Y, v1.Y),n_min(v0.Z, v1.Z));
}
//------------------------------------------------------------------------
inline 
float3 
float3::normalize(const float3& v)
{
	float fLength = v.length();
	// Will also work for zero-sized vectors, but will change nothing
	if ( fLength > epsilon )
	{
		float fInvLength = 1.0f / fLength;
		return float3( v.X * fInvLength, v.Y * fInvLength, v.Z * fInvLength);
	}
	else
	{
		return float3(0.0f, 0.0f, 0.0f);
	}
}
//------------------------------------------------------------------------
inline
bool 
float3::isNormalize(const float3& v)
{
	float ls = v.lengthsq();
	return n_fequal(1.0f, ls, epsilon);
}
//------------------------------------------------------------------------
inline 
float3 
float3::lt(const float3& v0, const float3& v1)
{
	float3 res;
	res.X = (v0.X < v1.X) ? 1.0f : 0.0f;
	res.Y = (v0.Y < v1.Y) ? 1.0f : 0.0f;
	res.Z = (v0.Z < v1.Z) ? 1.0f : 0.0f;
	return res;
}
//------------------------------------------------------------------------
inline 
float3
float3::le(const float3& v0, const float3& v1)
{
	float3 res;
	res.X = (v0.X <= v1.X) ? 1.0f : 0.0f;
	res.Y = (v0.Y <= v1.Y) ? 1.0f : 0.0f;
	res.Z = (v0.Z <= v1.Z) ? 1.0f : 0.0f;
	return res;
}
//------------------------------------------------------------------------
inline 
float3
float3::gt(const float3& v0, const float3& v1)
{
	float3 res;
	res.X = (v0.X > v1.X) ? 1.0f : 0.0f;
	res.Y = (v0.Y > v1.Y) ? 1.0f : 0.0f;
	res.Z = (v0.Z > v1.Z) ? 1.0f : 0.0f;
	return res;
}
//------------------------------------------------------------------------
inline 
float3
float3::ge(const float3& v0, const float3& v1)
{
	float3 res;
	res.X = (v0.X >= v1.X) ? 1.0f : 0.0f;
	res.Y = (v0.Y >= v1.Y) ? 1.0f : 0.0f;
	res.Z = (v0.Z >= v1.Z) ? 1.0f : 0.0f;
	return res;
}
//------------------------------------------------------------------------
inline float3 float3::perpendicular() const
{
	static const float fSquareZero = static_cast<float>(1e-06 * 1e-06);

	float3 perp = this->crossProduct(float3(1.0f, 0.0f, 0.0f));

	if (perp.lengthsq() < fSquareZero)
	{
		perp = this->crossProduct(float3(0.0f, 1.0f, 0.0f));
	}
	perp.normalise();

	return perp;
}
//------------------------------------------------------------------------
inline bool float3::compare(const float3 &v0, const float3 &v1, const float &tol)
{
	bool ret0 = n_fequal(v0.x(), v1.x(), tol);
	bool ret1 = n_fequal(v0.y(), v1.y(), tol);
	bool ret2 = n_fequal(v0.z(), v1.z(), tol);

	return ret0 && ret1 && ret2;
}

//------------------------------------------------------------------------
inline 
float
float3::operator [] ( const int i ) const
{
	assert( i < 3 );
	return *(&X+i);
}
//------------------------------------------------------------------------
inline 
float& 
float3::operator [] ( const int i )
{
	assert( i < 3 );
	return *(&X+i);
}
// namespace Math

}



#endif // __float3_H__
