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
#ifndef MATH_POLAR_H
#define MATH_POLAR_H
//------------------------------------------------------------------------------
/**
    @class Math::polar

    A polar coordinate inline class, consisting of 2 angles theta (latitude)
    and rho (longitude). Also offers conversion between cartesian and 
    polar space.

    Allowed range for theta is 0..180 degree (in rad!) and for rho 0..360 degree
    (in rad).

*/

#include "math/vector.h"
#include "math/float2.h"

//------------------------------------------------------------------------------
namespace Math
{
class polar
{
public:
    /// the default constructor
    polar();
    /// constructor, theta and rho args
    polar(float t, float r);
    /// constructor, normalized cartesian vector as arg
    polar(const vector& v);
    /// the copy constructor
    polar(const polar& src);
    /// the assignment operator
    void operator=(const polar& rhs);
    /// convert to normalized cartesian coords 
    vector get_cartesian() const;
    /// set to polar object
    void set(const polar& p);
    /// set to theta and rho
    void set(float t, float r);
    /// set to cartesian 
    void set(const vector& v);

    float theta;
    float rho;
};

//------------------------------------------------------------------------------
/**
*/
inline
polar::polar() :
    theta(0.0f),
    rho(0.0f)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
polar::polar(float t, float r) :
    theta(t),
    rho(r)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
polar::polar(const vector& v)
{
    this->set(v);
}

//------------------------------------------------------------------------------
/**
*/
inline
polar::polar(const polar& src) :
    theta(src.theta),
    rho(src.rho)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline void
polar::operator=(const polar& rhs)
{
    this->theta = rhs.theta;
    this->rho = rhs.rho;
}

//------------------------------------------------------------------------------
/**
*/
inline void
polar::set(const polar& p)
{
    this->theta = p.theta;
    this->rho = p.rho;
}

//------------------------------------------------------------------------------
/**
*/
inline void
polar::set(float t, float r)
{
    this->theta = t;
    this->rho = r;
}

//------------------------------------------------------------------------------
/**
    Convert cartesian to polar.
*/
inline void
polar::set(const vector& vec)
{
    vector normVec3d = vector::normalize(vec);
    this->theta = n_acos(normVec3d.y());

    // build a normalized 2d vector of the xz component
    float2 normVec2d(normVec3d.x(), normVec3d.z());
    if (normVec2d.length() > TINY)
    {
        normVec2d = float2::normalize(normVec2d);
    }
    else
    {
        normVec2d.set(1.0f, 0.0f);
    }

    // adjust dRho based on the quadrant we are in
    if ((normVec2d.x() >= 0.0f) && (normVec2d.y() >= 0.0f))
    {
        // quadrant 1
        this->rho = asinf(normVec2d.x());
    }
    else if ((normVec2d.x() < 0.0f) && (normVec2d.y() >= 0.0f))
    {
        // quadrant 2 
        this->rho = asinf(normVec2d.y()) + n_deg2rad(270.0f);
    }
    else if ((normVec2d.x() < 0.0f) && (normVec2d.y() < 0.0f))
    {
        // quadrant 3
        this->rho = asinf(-normVec2d.x()) + n_deg2rad(180.0f);
    }
    else
    {
        // quadrant 4
        this->rho = asinf(-normVec2d.y()) + n_deg2rad(90.0f);
    }
}
    
//------------------------------------------------------------------------------
/**
    Convert polar to cartesian.
*/
inline vector
polar::get_cartesian() const
{
    float sinTheta = sinf(this->theta);
    float cosTheta = cosf(this->theta);
    float sinRho   = sinf(this->rho);
    float cosRho   = cosf(this->rho);
    vector v(sinTheta * sinRho, cosTheta, sinTheta * cosRho);
    return v;
}

} // namespace Math
//------------------------------------------------------------------------------
#endif