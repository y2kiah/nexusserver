/*----==== FASTMATH.H ====----
	Author:	Jeff Kiah
	Date:	12/7/2007
	Rev:	11/16/2010
	Notes:	removed sin/tan lookup tables - native call faster than memory access
----------------------------*/

#pragma once

#include <cmath>
#include <crtdbg.h>
#include <climits>
#include <float.h>
#include "../Utility/Typedefs.h"

///// DEFINES /////

#define PIf				3.14159265358979323846f		// PI
#define TWO_PIf			6.28318530717958647692f		// PI * 2
#define	HALF_PIf		1.57079632679489661923f		// PI / 2
#define DEGTORADf		0.01745329252f				// PI / 180
#define RADTODEGf		57.29577951308f				// 180 / PI
#define HUGE_VALf		(float)(HUGE_VAL)
#define LN_HALFf		-0.69314718055994530941f	// ln(.5)
#define INV_LN_HALFf	-1.44269504088896340736f	// 1 / ln(.5)
#define LOG_HALFf		-0.30102999566398119521f	// log(.5)
#define INV_LOG_HALFf	-3.32192809488736234787f	// 1 / log(.5)
#define INFf			std::numeric_limits<float>::infinity()	// representation of infinity
#define EPSf			std::numeric_limits<float>::epsilon()	// representation smallest value

#define PI				3.14159265358979323846
#define TWO_PI			6.28318530717958647692
#define	HALF_PI			1.57079632679489661923
#define DEGTORAD		0.01745329252
#define RADTODEG		57.29577951308
#define INF				std::numeric_limits<double>::infinity()
#define EPS				std::numeric_limits<double>::epsilon()

// reinterpret a float as an int
#define fpBits(f)			(*reinterpret_cast<const int*>(&(f))) 
// get the 23 bits of mantissa without the implied bit
#define fpPureMantissa(f)	((fpBits(f)&0x7fffff))

///// STRUCTURES /////

/*=============================================================================
	Collection of static math related functions
=============================================================================*/
class FastMath {
	public:
		///// FUNCTIONS /////

		// Get angles into usable range (e.g. 0 to 359)
		inline static float	degToRange(float degrees);
		inline static int	degToRange(int degrees);
		inline static float	radToRange(float radians);

		// Fast floating point to integer conversions
		inline static int	f2iTrunc(const float f);
		inline static int	d2iTrunc(const double d);
		inline static int	f2iFloor(const float f);
		inline static int	d2iFloor(const double d);
		inline static int	f2iCeil(const float f);
		inline static int	d2iCeil(const double d);
		inline static int	f2iRound(const float f);
		inline static int	d2iRound(const double d);
		inline static int	f2iRoundFast(const float f);
		inline static int	d2iRoundFast(const double d);

		// Fast math.h replacements
		inline static long	sqrti(long x);
		static int			abs(int x)		{ return (x > 0) ? x : -x; }
		static float		abs(float x)	{ return (x > 0.0f) ? x : -x; }
		static double		abs(double x)	{ return (x > 0.0) ? x : -x; }
		template <typename T> static T abs(T x) { return (x > 0) ? x : -x; }

		// Interpolation functions
		static float	lerp(float a, float b, float t)	{ return a + t * (b - a); }
		static float	sCurve(float t) 				{ return t * t * (3.0f - 2.0f * t); } // Cubic S-curve = 3t^2 - 2t^3 : 2nd derivative is discontinuous at t=0 and t=1 causing visual artifacts at boundaries
		static float	qCurve(float t) 				{ return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); } // Quintic curve
		static float	cosCurve(float t)				{ return (1.0f - cosf(t * PIf)) * 0.5f; } // Cosine curve
		
		static float	step(float a, float x)			{ return static_cast<float>(x >= a); }
		static float	boxStep(float a, float b, float x) { _ASSERTE(b!=a); return clamp(0.0f, 1.0f, (x-a)/(b-a)); }
		static float	pulse(float a, float b, float x) { return static_cast<float>((x >= a) - (x >= b)); }

		// Other shortcuts
		template <class T> static T minVal(T a, T b)	{ return (a < b ? a : b); }
		template <class T> static T maxVal(T a, T b)	{ return (a > b ? a : b); }
		static float	floor(float a)					{ return static_cast<float>(f2iFloor(a)); }
		static float	ceil(float a)					{ return static_cast<float>(f2iCeil(a)); }
		template <class T> static T clamp(T a, T b, T x) { return (x < a ? a : (x > b ? b : x)); }
		static float	bias(float a, float b)			{ return powf(a, logf(b) * INV_LN_HALFf); }
		static float	gamma(float a, float g)			{ return powf(a, 1.0f / g); }
		static float	expose(float l, float k)		{ return (1.0f - expf(-l * k)); }

		// utility functions
		template <class T> inline static int	highestBitSet(T x);
		template <class T> inline static int	lowestBitSet(T x);
		template <class T> inline static bool	isPowerOfTwo(T x);
		inline static uint						roundUpToPowerOfTwo(uint x);
};


///// INLINE FUNCTIONS /////
inline float FastMath::degToRange(float degrees)
{
	while (degrees >= 360.0f) { degrees -= 360.0f; }
	while (degrees < 0.0f) { degrees += 360.0f; }
	return degrees;
}

inline int FastMath::degToRange(int degrees)
{
	while (degrees >= 360) { degrees -= 360; }
	while (degrees < 0) { degrees += 360; }
	return degrees;
}

inline float FastMath::radToRange(float radians)
{
	while (radians >= TWO_PIf) { radians -= TWO_PIf; }
	while (radians < 0.0f) { radians += TWO_PIf; }
	return radians;
}

inline long FastMath::sqrti(long x)
{
	if (x < 1) return 0;
	// Load the binary constant 01 00 00 ... 00, where the number of zero bits to the
	// right of the single one bit is even, and the one bit is as far left as is consistant with that condition
	long squaredBit = (long)((((unsigned long)~0L) >> 1) & ~(((unsigned long)~0L) >> 2));
	// Form bits of the answer
	long root = 0;
	while (squaredBit > 0) {
		if (x >= (squaredBit | root)) {
			x -= (squaredBit | root);
			root >>= 1; root |= squaredBit;
		} else {
			root >>= 1;
		}
		squaredBit >>= 2; 
	}
	return root;
}

// Use these to in place of standard "C" style (int) cast
inline int FastMath::f2iTrunc(const float f)
{
	_ASSERTE(abs(f) < static_cast<float>(INT_MAX / 2) + 1.0f);
	
	const float	roundTowardsMinusInfinity = -0.5f;
	int i;
	__asm {
		fld		f
		fadd	st, st (0)
		fabs
		fadd	roundTowardsMinusInfinity
		fistp	i
		sar		i, 1
	}
	return (f < 0) ? -i : i;
}

inline int FastMath::d2iTrunc(const double d)
{
	_ASSERTE(abs(d) < static_cast<double>(INT_MAX / 2) + 1.0);

	const float	roundTowardsMinusInfinity = -0.5f;
	int i;
	__asm {
		fld		d
		fadd	st, st (0)
		fabs
		fadd	roundTowardsMinusInfinity
		fistp	i
		sar		i, 1
	}
	return (d < 0) ? -i : i;
}

inline int FastMath::f2iFloor(const float f)
{
	_ASSERTE(abs(f) < static_cast<float>(INT_MAX / 2) + 1.0f);
	
	const float	roundTowardsMinusInfinity = -0.5f;
	int i;
	__asm {
		fld		f
		fadd	st, st (0)
		fadd	roundTowardsMinusInfinity
		fistp	i
		sar		i, 1
	}
	return i;
}

inline int FastMath::d2iFloor(const double d)
{
	_ASSERTE(abs(d) < static_cast<double>(INT_MAX / 2) + 1.0);
	
	const float	roundTowardsMinusInfinity = -0.5f;
	int i;
	__asm {
		fld		d
		fadd	st, st (0)
		fadd	roundTowardsMinusInfinity
		fistp	i
		sar		i, 1
	}
	return i;
}

inline int FastMath::f2iCeil(const float f)
{
	_ASSERTE(abs(f) < static_cast<float>(INT_MAX / 2) + 1.0f);

	const float	roundTowardsPlusInfinity = -0.5f;
	int i;
	__asm {
		fld		f
		fadd	st, st (0)
		fsubr	roundTowardsPlusInfinity
		fistp	i
		sar		i, 1
	}
	return -i;
}

inline int FastMath::d2iCeil(const double d)
{
	_ASSERTE(abs(d) < static_cast<double>(INT_MAX / 2) + 1.0);

	const float	roundTowardsPlusInfinity = -0.5f;
	int i;
	__asm {
		fld		d
		fadd	st, st (0)
		fsubr	roundTowardsPlusInfinity
		fistp	i
		sar		i, 1
	}
	return -i;
}

inline int FastMath::f2iRound(const float f)
{
	_ASSERTE(abs(f) < static_cast<float>(INT_MAX / 2) + 1.0f);

	const float	roundToNearest = 0.5f;
	int i;
	__asm {
		fld		f
		fadd	st, st (0)
		fadd	roundToNearest
		fistp	i
		sar		i, 1
	}
	return i;
}

inline int FastMath::d2iRound(const double d)
{
	_ASSERTE(abs(d) < static_cast<double>(INT_MAX / 2) + 1.0);

	const float	roundToNearest = 0.5f;
	int i;
	__asm {
		fld		d
		fadd	st, st (0)
		fadd	roundToNearest
		fistp	i
		sar		i, 1
	}
	return i;
}

// If the FPU rounding mode is set to truncate these will truncate
// FPU rounding mode set to round to nearest int, the following 2 round to closest even integer
// These are inconsistent in cases of numbers directly between two integers 
inline int FastMath::d2iRoundFast(const double d)
{
	int i;
	__asm {
		fld		d
		fistp	i
	}
	return i;
}

inline int FastMath::f2iRoundFast(const float f)
{
	int i;
	__asm {
		fld		f
		fistp	i
	}
	return i;
}

// Returns the index of the highest bit set in x
template <class T>
inline int FastMath::highestBitSet(T x)
{
	register int result;
    _ASSERTE(x);			// zero is invalid input!
    _ASSERTE(sizeof(T)==4);	// 32bit data only!
    _asm bsr eax, x
    _asm mov result, eax
	return result;
}

template <>
inline int FastMath::highestBitSet(uchar x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dl, x		// copy into a 32bit reg
    _asm and edx, 0xff	// keep only the bits we want
    _asm bsr eax, edx	// perform the scan
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::highestBitSet(char x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dl, x
    _asm and edx, 0xff
    _asm bsr eax, edx
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::highestBitSet(ushort x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dx, x
    _asm and edx, 0xffff
    _asm bsr eax, edx
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::highestBitSet(short x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dx, x
    _asm and edx, 0xffff
    _asm bsr eax, edx
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::highestBitSet(float f)
{
	register int result;
	register uint x = fpBits(f);
    _ASSERTE(x);
    _asm bsr eax, x
    _asm mov result, eax
	return result;
}

// Returns the index of the lowest bit set in x
template<class T>
inline int FastMath::lowestBitSet(T x)
{
	register int result;
    _ASSERTE(x);
    _ASSERTE(sizeof(T)==4);
    _asm bsf eax, x
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::lowestBitSet(uchar x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dl, x		// copy into a 32bit reg
    _asm and edx, 0xff	// keep only the bits we want
    _asm bsf eax, edx	// perform the scan
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::lowestBitSet (char x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dl, x
    _asm and edx, 0xff
    _asm bsf eax, edx
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::lowestBitSet(ushort x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dx, x
    _asm and edx, 0xffff
    _asm bsf eax, edx
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::lowestBitSet(short x)
{
	register int result;
    _ASSERTE(x);
    _asm mov dx, x
    _asm and edx, 0xffff
    _asm bsf eax, edx
    _asm mov result, eax
	return result;
}

template<>
inline int FastMath::lowestBitSet(float f)
{
	register int result;
	register uint x = fpBits(f);
    _ASSERTE(x);
    _asm bsf eax, x
    _asm mov result, eax
	return result;
}

// Returns true if the input value is a power-of-two (1,2,4,8,16,etc.)
template<class T>
inline bool FastMath::isPowerOfTwo(T x)
{
    // If the value is greater than zero and has only one bit set it must be a power of two
    return (x > 0 && highestBitSet(x) == lowestBitSet(x));
}

template<>
inline bool FastMath::isPowerOfTwo(float x)
{
	// For floating-point values, we know the value is a power-of-two if the mantissa is zero
	// (not including the implied bit)
    return (!fpPureMantissa(x));
}

inline uint FastMath::roundUpToPowerOfTwo(uint x)
{
	uint xTry = 1;
	while (xTry < x) {
		xTry <<= 1;
	}
	return xTry;
}