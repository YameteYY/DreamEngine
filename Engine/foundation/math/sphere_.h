/****************************************************************************
Copyright (c) 2004, Radon Labs GmbH
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

#pragma once
#ifndef MATH_SPHERE_H
#define MATH_SPHERE_H
//------------------------------------------------------------------------------
/**
    @class Math::sphere

    A 3-dimensional sphere.

    (C) 2004 RadonLabs GmbH
*/
#include "math/vector.h"
#include "math/point.h"
#include "math/bbox.h"
#include "math/matrix44.h"
#include "math/rectangle.h"
#include "math/clipstatus.h"

//------------------------------------------------------------------------------
namespace Math
{
class sphere 
{
public:
    /// default constructor
    sphere();
    /// pos/radius constructor
    sphere(const point& _p, float _r);
    /// x,y,z,r constructor
    sphere(float _x, float _y, float _z, float _r);
    /// copy constructor
    sphere(const sphere& rhs);
    /// set position and radius
    void set(const point& _p, float _r);
    /// set x,y,z, radius
    void set(float _x, float _y, float _z, float _r);
    /// return true if box is completely inside sphere
	/// transform bounding box
	void transform(const matrix44& m);

    bool inside(const bbox& box) const;
    /// check if 2 spheres overlap
    bool intersects(const sphere& s) const;
    /// check if sphere intersects box
    bool intersects(const bbox& box) const;
    /// check if 2 moving sphere have contact
    bool intersect_sweep(const vector& va, const sphere& sb, const vector& vb, float& u0, float& u1) const;
    /// project sphere to screen rectangle (right handed coordinate system)
    rectangle<float> project_screen_rh(const matrix44& modelView, const matrix44& projection, float nearZ) const;
    /// get clip status of box against sphere
    ClipStatus::Type clipstatus(const bbox& box) const;
        
    point p;
    float r;
};

//------------------------------------------------------------------------------
/**
*/
inline
sphere::sphere() :
    r(1.0f)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
sphere::sphere(const point& _p, float _r) :
    p(_p),
    r(_r)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
sphere::sphere(float _x, float _y, float _z, float _r) :
    p(_x, _y, _z),
    r(_r)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
sphere::sphere(const sphere& rhs) :
    p(rhs.p),
    r(rhs.r)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline void
sphere::set(const point& _p, float _r)
{
    this->p = _p;
    this->r = _r;
}

//------------------------------------------------------------------------------
/**
*/
inline void
sphere::set(float _x, float _y, float _z, float _r)
{
    this->p.set(_x, _y, _z);
    this->r = _r;
}

//------------------------------------------------------------------------------
/**
*/
inline bool 
sphere::intersects(const sphere& s) const 
{
    vector d(s.p - p);
    float rsum = s.r + r;
    return (d.lengthsq() <= (rsum * rsum));
}

//------------------------------------------------------------------------------
/**
    Return true if the bounding box is inside the sphere.
*/
inline bool
sphere::inside(const bbox& box) const
{
    vector v(this->r, this->r, this->r);
    point pmin(this->p - v);
    point pmax(this->p + v);
    bool lt = float4::less3_all(box.pmin, pmin);
    bool ge = float4::greaterequal3_all(box.pmax, pmax);
    return lt && ge;
}

//------------------------------------------------------------------------------
/**
    Get the clip status of a box against this sphere. Inside means: the
    box is completely inside the sphere.
*/
inline
ClipStatus::Type
sphere::clipstatus(const bbox& box) const
{
    if (this->inside(box)) return ClipStatus::Inside;
    else if (this->intersects(box)) return ClipStatus::Clipped;
    else return ClipStatus::Outside;
}

} // namespace Math
//------------------------------------------------------------------------------
#endif
