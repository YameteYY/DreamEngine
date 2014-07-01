/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
-----------------------------------------------------------------------------
*/
#ifndef __OgrePrerequisites_H__
#define __OgrePrerequisites_H__
#include <numeric>
#include <limits>
#define FORCEINLINE inline
#include <assert.h>
#include <algorithm>

#ifndef TINY
#define TINY (0.0000001f)
#endif
#define N_TINY TINY

#define N_FLOAT32_MAX 3.402823E+38f

#define N_FLOAT32_MIN -3.402823E+38f

const float N_INFINITY = 1e38f;

namespace Ogre
{
	typedef float Real;
	class Degree;
	class Vector3;
	class Vector4;
	class Matrix3;
	class Matrix4;
}
__forceinline float
n_cot(float x)
{
	return float(1.0) / tanf(x);
}
//------------------------------------------------------------------------
template<typename T>
T n_max(T a, T b)
{
	return (a > b) ? a : b;
}
//------------------------------------------------------------------------
template<typename T>
T n_min(T a, T b)
{
	return (a < b) ? a : b;
}
__forceinline bool
	n_isNaN(float f)
{
	// from Ogre;
	// std::isnan() is C99, not supported by all compilers
	// However NaN always fails this next test, no other number does.
	return f != f;
}
__forceinline bool
	n_fequal(float f0, float f1, float tol)
{
	float f = f0 - f1;
	return ((f > (-tol)) && (f < tol));
}

__forceinline float
	n_lerp(float x, float y, float l)
{
	return x + l * (y - x);
}

#endif 