/****************************************************************************
Copyright (c) 2007, Radon Labs GmbH
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


namespace Math
{
typedef float float;

const float LN_2 = 0.693147180559945f;

#ifndef PI
#define PI (3.1415926535897932384626433832795028841971693993751)
#endif
// the half circle
#ifndef N_PI
#define N_PI (float(PI))
#endif

//------------------------------------------------------------------------------
/**
    Return a pseudo random number between 0 and 1.
*/
__forceinline float 
n_rand()
{
    return float(rand()) / float(RAND_MAX);
}

//------------------------------------------------------------------------------
/**
    Return a pseudo random number between min and max.
*/
__forceinline float 
n_rand(float min, float max)
{
	float unit = float(rand()) / RAND_MAX;
	float diff = max - min;
	return min + unit * diff;
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
sinf(float x)
{
	return sinf(x);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
cosf(float x)
{
	return cosf(x);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
n_tan(float x)
{
    return tanf(x);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
asinf(float x)
{
	return asinf(x);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
n_acos(float x)
{
	return acosf(x);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
n_atan(float x)
{
    return atanf(x);
}
//------------------------------------------------------------------------------
/**
*/
__forceinline float
	atan2f(float x, float y)
{
	return atan2f(x, y);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float 
sqrt(float x)
{
    return sqrtf(x);
}

//------------------------------------------------------------------------------
/**
    Chop float to int.
*/
__forceinline int 
n_fchop(float f)
{
    /// @todo type cast to int is slow!
    return int(f);
}

//------------------------------------------------------------------------------
/**
    log2() function.
*/
__forceinline float 
n_log2(float f) 
{ 
    return logf(f) / LN_2; 
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
n_exp(float x)
{
    return expf(x);
}

//------------------------------------------------------------------------------
/**
    Round float to integer.
*/
__forceinline int 
n_frnd(float f)
{
    return n_fchop(floorf(f + 0.5f));
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
n_fmod(float x, float y)
{
    return fmodf(x, y);
}

//------------------------------------------------------------------------------
/**
*/
__forceinline float
n_pow(float x, float y)
{
    return powf(x, y);
}

//------------------------------------------------------------------------------
/**
	get logarithm of x
*/
__forceinline float
n_log(float x)
{
    return logf(x);
}

//------------------------------------------------------------------------------
/**
	get sign of x
*/
__forceinline float
n_sign (float x)
{
	if ( x > 0.0 )
		return 1.0;

	if ( x < 0.0 )
		return -1.0;
	return 0.0;
}

__forceinline int
	n_nexPowerOfTwo (int x)
{
	int base = 1;

	while( base < x)
	{
		base = base << 1;
	}
	return base;
}

__forceinline bool
	n_IsPowerOfTwo(int x)
{
	return !(x&(x-1));
}

__forceinline int
	n_logTwoInt (int x)
{
	int n = 0;

	while (x > 1)
	{
		x = x >> 1;
		n++;
	}

	return n;
}

} // namespace Math
//------------------------------------------------------------------------------



    