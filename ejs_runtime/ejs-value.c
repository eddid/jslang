/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_h_
#define _ejs_h_

#include <stdio.h>
#include <stdlib.h>
#include "ejs-log.h"
#include "ejs-string.h"
#include "ejs-closureenv.h"

#include <math.h>
#include <float.h>
#if defined(WIN32)
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
inline double nan(const char *str) { return NAN; }
inline bool isinf(double num) { return !_finite(num) && !_isnan(num); }
inline bool isnan(double num) { return !!_isnan(num); }
inline int isfinite(double x) { return _finite(x); }
inline double round(double num)
{
    double integer = ceil(num);
    if (num > 0)
        return integer - num > 0.5 ? integer - 1.0 : integer;
    return integer - num >= 0.5 ? integer - 1.0 : integer;
}
inline double log2(double num)
{
    // This constant is roughly M_LN2, which is not provided by default on Windows.
    return log(num) / 0.693147180559945309417232121458176568;
}
inline double log1p(double x)
{
    return ((1+x)-1) ? log(1+x)*(x/((1+x)-1)) : x;
}
inline double asinh(double value)
{
    double returned;

    if(value>0)
       returned = log(value + sqrt(value * value + 1));
    else
       returned = -log(-value + sqrt(value * value + 1));
   
    return(returned);
}
inline double acosh(double value)
{
    return log(value + sqrt(value * value - 1));
}
inline double atanh(double value)
{
    return (log(1+value) - log(1-value))/2;
}
inline double expm1(double value)
{
    return exp(value)-1;
}
inline double trunc(double value)
{
    return (value>0) ? floor(value) : ceil(value);
}
inline double cbrt(double value)
{
    if (fabs(value) < DBL_EPSILON) return 0.0;

    if (value > 0.0) return pow(value, 1.0/3.0);

    return -pow(-value, 1.0/3.0);
}
#define FP_NAN		0x0100
#define FP_NORMAL	0x0400
#define FP_INFINITE	(FP_NAN | FP_NORMAL)
#define FP_ZERO		0x4000
#define FP_SUBNORMAL	(FP_NORMAL | FP_ZERO)
typedef union __mingw_dbl_type_t {
  double x;
  unsigned long long val;
  struct {
    unsigned int low, high;
  } lh;
} __mingw_dbl_type_t;
int fpclassify (double _x)
{
    __mingw_dbl_type_t hlp;
    unsigned int l, h;

    hlp.x = _x;
    h = hlp.lh.high;
    l = hlp.lh.low | (h & 0xfffff);
    h &= 0x7ff00000;
    if ((h | l) == 0)
      return FP_ZERO;
    if (!h)
      return FP_SUBNORMAL;
    if (h == 0x7ff00000)
      return (l ? FP_NAN : FP_INFINITE);
    return FP_NORMAL;
}
int ffs(int field)
{
    static const int index[] = { 1, 2, 29, 3, 30, 15, 25, 4, 31, 23, 21, 
    16, 26, 18, 5, 9, 32, 28, 14, 24, 22, 20, 17, 8, 27, 13, 19, 7, 12, 6, 11, 10 };
    unsigned int w = field;
    if (w == 0)
        return (0);
    w &= -w;
    w *= 125613361U;
    w >>= 27;
    return index[w];
}

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  __mingw_dbl_type_t ew_u;					\
  ew_u.x = (d);						\
  (ix0) = ew_u.lh.high;					\
  (ix1) = ew_u.lh.low;					\
} while (0)

#define INSERT_WORDS(d,ix0,ix1)					\
do {								\
  __mingw_dbl_type_t iw_u;					\
  iw_u.lh.high = (ix0);					\
  iw_u.lh.low = (ix1);					\
  (d) = iw_u.x;						\
} while (0)
double nextafter(double x, double y)
{
	volatile double t;
	int hx,hy,ix,iy;
	unsigned int lx,ly;

	EXTRACT_WORDS(hx,lx,x);
	EXTRACT_WORDS(hy,ly,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffff;		/* |y| */

	if(((ix>=0x7ff00000)&&((ix-0x7ff00000)|lx)!=0) ||   /* x is nan */
	   ((iy>=0x7ff00000)&&((iy-0x7ff00000)|ly)!=0))     /* y is nan */
	   return x+y;
	if(x==y) return y;		/* x=y, return y */
	if((ix|lx)==0) {			/* x == 0 */
	    INSERT_WORDS(x,hy&0x80000000,1);	/* return +-minsubnormal */
	    t = x*x;
	    if(t==x) return t; else return x;	/* raise underflow flag */
	}
	if(hx>=0) {				/* x > 0 */
	    if(hx>hy||((hx==hy)&&(lx>ly))) {	/* x > y, x -= ulp */
		if(lx==0) hx -= 1;
		lx -= 1;
	    } else {				/* x < y, x += ulp */
		lx += 1;
		if(lx==0) hx += 1;
	    }
	} else {				/* x < 0 */
	    if(hy>=0||hx>hy||((hx==hy)&&(lx>ly))){/* x < y, x -= ulp */
		if(lx==0) hx -= 1;
		lx -= 1;
	    } else {				/* x > y, x += ulp */
		lx += 1;
		if(lx==0) hx += 1;
	    }
	}
	hy = hx&0x7ff00000;
	if(hy>=0x7ff00000) return x+x;	/* overflow  */
	if(hy<0x00100000) {		/* underflow */
	    t = x*x;
	    if(t!=x) {		/* raise underflow flag */
	        INSERT_WORDS(y,hx,lx);
		return y;
	    }
	}
	INSERT_WORDS(x,hx,lx);
	return x;
}
#endif

static EJS_ALWAYS_INLINE EJSBool
EJSDOUBLE_IS_NEGZERO(double d)
{
    if (d != 0)
        return EJS_FALSE;
    ejsval x;
    x.asDouble = d;
    return (x.asIntPtr[jsValue32TagPart] & EJSDOUBLE_HI32_SIGNBIT) != 0;
}

static EJS_ALWAYS_INLINE EJSBool
EJSDOUBLE_IS_INT32(double d, int32_t* pi)
{
    if (EJSDOUBLE_IS_NEGZERO(d))
        return EJS_FALSE;
    return d == (*pi = (int32_t)d);
}

#if EJS_BITS_PER_WORD == 32

static EJS_ALWAYS_INLINE EJSValueTag
EJSVAL_TO_TAG(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart];
}

/*
 * N.B. GCC, in some but not all cases, chooses to emit signed comparison of
 * EJSValueTag even though its underlying type has been forced to be uint32_t.
 * Thus, all comparisons should explicitly cast operands to uint32_t.
 */

static EJS_ALWAYS_INLINE ejsval
BUILD_EJSVAL(EJSValueTag tag, uint32_t payload)
{
    ejsval l;
    l.asIntPtr[jsValue32TagPart] = (uint32_t)tag;
    l.asIntPtr[jsValue32PayloadPart] = payload;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_DOUBLE_IMPL(ejsval l)
{
    return (uint32_t)l.asIntPtr[jsValue32TagPart] <= (uint32_t)EJSVAL_TAG_CLEAR;
}

static EJS_ALWAYS_INLINE ejsval
DOUBLE_TO_EJSVAL_IMPL(double d)
{
    ejsval l;
    l.asDouble = d;
    EJS_ASSERT(EJSVAL_IS_DOUBLE_IMPL(l));
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_INT32_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_INT32;
}

static EJS_ALWAYS_INLINE int32_t
EJSVAL_TO_INT32_IMPL(ejsval l)
{
    return (int32_t)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE ejsval
INT32_TO_EJSVAL_IMPL(int32_t i)
{
    ejsval l;
    l.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_INT32;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)i;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_NUMBER_IMPL(ejsval l)
{
    EJSValueTag tag = l.asIntPtr[jsValue32TagPart];
    EJS_ASSERT(tag != EJSVAL_TAG_CLEAR);
    return (uint32_t)tag <= (uint32_t)EJSVAL_TAG_INT32;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_UNDEFINED_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_UNDEFINED;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_STRING_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_STRING;
}

static EJS_ALWAYS_INLINE ejsval
STRING_TO_EJSVAL_IMPL(EJSPrimString *str)
{
    ejsval l;
    EJS_ASSERT(str);
    l.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_STRING;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)str;
    return l;
}

static EJS_ALWAYS_INLINE EJSPrimString *
EJSVAL_TO_STRING_IMPL(ejsval l)
{
    return (EJSPrimString *)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_CLOSUREENV_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_CLOSUREENV;
}

static EJS_ALWAYS_INLINE ejsval
CLOSUREENV_TO_EJSVAL_IMPL(EJSClosureEnv *env)
{
    ejsval l;
    EJS_ASSERT(env);
    l.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_CLOSUREENV;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)env;
    return l;
}

static EJS_ALWAYS_INLINE EJSClosureEnv *
EJSVAL_TO_CLOSUREENV_IMPL(ejsval l)
{
    return (EJSClosureEnv *)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_BOOLEAN_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_BOOLEAN;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_TO_BOOLEAN_IMPL(ejsval l)
{
    return (EJSBool)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE ejsval
BOOLEAN_TO_EJSVAL_IMPL(EJSBool b)
{
    ejsval l;
    EJS_ASSERT(b == EJS_TRUE || b == EJS_FALSE);
    l.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_BOOLEAN;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)b;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_MAGIC_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_MAGIC;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_OBJECT_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_OBJECT;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_PRIMITIVE_IMPL(ejsval l)
{
    return (uint32_t)l.asIntPtr[jsValue32TagPart] < (uint32_t)EJSVAL_TAG_OBJECT;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_OBJECT_OR_NULL_IMPL(ejsval l)
{
    EJS_ASSERT((uint32_t)l.asIntPtr[jsValue32TagPart] <= (uint32_t)EJSVAL_TAG_OBJECT);
    return (uint32_t)l.asIntPtr[jsValue32TagPart] >= (uint32_t)EJSVAL_TAG_NULL
;
}

static EJS_ALWAYS_INLINE EJSObject *
EJSVAL_TO_OBJECT_IMPL(ejsval l)
{
    return (EJSObject *)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE ejsval
OBJECT_TO_EJSVAL_IMPL(EJSObject *obj)
{
    ejsval l;
    EJS_ASSERT(obj);
    l.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_OBJECT;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)obj;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_NULL_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_NULL;
}

static EJS_ALWAYS_INLINE ejsval
PRIVATE_PTR_TO_EJSVAL_IMPL(const void *ptr)
{
    ejsval l;
    EJS_ASSERT(((uint32_t)ptr & 1) == 0);
    l.asIntPtr[jsValue32TagPart] = (EJSValueTag)0;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)ptr;
    EJS_ASSERT(EJSVAL_IS_DOUBLE_IMPL(l));
    return l;
}

static EJS_ALWAYS_INLINE void *
EJSVAL_TO_PRIVATE_PTR_IMPL(ejsval l)
{
    return (void *)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_GCTHING_IMPL(ejsval l)
{
    /* gcc sometimes generates signed < without explicit casts. */
    return (uint32_t)l.asIntPtr[jsValue32TagPart] >= (uint32_t)EJSVAL_TAG_STRING;
}

static EJS_ALWAYS_INLINE void *
EJSVAL_TO_GCTHING_IMPL(ejsval l)
{
    return (void *)l.asIntPtr[jsValue32PayloadPart];
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_TRACEABLE_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_STRING || l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_CLOSUREENV || l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_OBJECT;
}

static EJS_ALWAYS_INLINE uint32_t
EJSVAL_TRACE_KIND_IMPL(ejsval l)
{
    return (uint32_t)(EJSBool)EJSVAL_IS_STRING_IMPL(l);
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_SPECIFIC_INT32_IMPL(ejsval l, int32_t i32)
{
    return l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_INT32 && l.asIntPtr[jsValue32PayloadPart] == (uint32_t)i32;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_SPECIFIC_BOOLEAN(ejsval l, EJSBool b)
{
    return (l.asIntPtr[jsValue32TagPart] == EJSVAL_TAG_BOOLEAN) && (l.asIntPtr[jsValue32PayloadPart] == (uint32_t)b);
}

static EJS_ALWAYS_INLINE ejsval
MAGIC_TO_EJSVAL_IMPL(EJSWhyMagic why)
{
    ejsval l;
    l.asIntPtr[jsValue32TagPart] = EJSVAL_TAG_MAGIC;
    l.asIntPtr[jsValue32PayloadPart] = (uint32_t)why;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_SAME_TYPE_IMPL(ejsval lhs, ejsval rhs)
{
    EJSValueTag ltag = lhs.asIntPtr[jsValue32TagPart], rtag = rhs.asIntPtr[jsValue32TagPart];
    return ltag == rtag || (ltag < EJSVAL_TAG_CLEAR && rtag < EJSVAL_TAG_CLEAR);
}

static EJS_ALWAYS_INLINE ejsval
PRIVATE_UINT32_TO_EJSVAL_IMPL(uint32_t ui)
{
    ejsval l;
    l.asIntPtr[jsValue32TagPart] = (EJSValueTag)0;
    l.asIntPtr[jsValue32PayloadPart] = ui;
    EJS_ASSERT(EJSVAL_IS_DOUBLE_IMPL(l));
    return l;
}

static EJS_ALWAYS_INLINE uint32_t
EJSVAL_TO_PRIVATE_UINT32_IMPL(ejsval l)
{
    return l.asIntPtr[jsValue32PayloadPart];
}

#elif EJS_BITS_PER_WORD == 64

static EJS_ALWAYS_INLINE EJSValueTag
EJSVAL_TO_TAG(ejsval l)
{
    return (EJSValueTag)((uint64_t)l.asInt64 >> EJSVAL_TAG_SHIFT);
}

static EJS_ALWAYS_INLINE ejsval
BUILD_EJSVAL(EJSValueTag tag, uint64_t payload)
{
    ejsval l;
    l.asInt64 = (((uint64_t)(uint32_t)tag) << EJSVAL_TAG_SHIFT) | payload;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_DOUBLE_IMPL(ejsval l)
{
    return l.asInt64 <= EJSVAL_SHIFTED_TAG_MAX_DOUBLE;
}

static EJS_ALWAYS_INLINE ejsval
DOUBLE_TO_EJSVAL_IMPL(double d)
{
    ejsval l;
    l.asDouble = d;
    EJS_ASSERT(l.asInt64 <= EJSVAL_SHIFTED_TAG_MAX_DOUBLE);
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_INT32_IMPL(ejsval l)
{
    return (uint32_t)(l.asInt64 >> EJSVAL_TAG_SHIFT) == EJSVAL_TAG_INT32;
}

static EJS_ALWAYS_INLINE int32_t
EJSVAL_TO_INT32_IMPL(ejsval l)
{
    return (int32_t)l.asInt64;
}

static EJS_ALWAYS_INLINE ejsval
INT32_TO_EJSVAL_IMPL(int32_t i32)
{
    ejsval l;
    l.asInt64 = ((uint64_t)(uint32_t)i32) | EJSVAL_SHIFTED_TAG_INT32;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_NUMBER_IMPL(ejsval l)
{
    return l.asInt64 < EJSVAL_SHIFTED_TAG_UNDEFINED;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_UNDEFINED_IMPL(ejsval l)
{
    return l.asInt64 == EJSVAL_SHIFTED_TAG_UNDEFINED;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_STRING_IMPL(ejsval l)
{
    return (uint32_t)(l.asInt64 >> EJSVAL_TAG_SHIFT) == EJSVAL_TAG_STRING;
}

static EJS_ALWAYS_INLINE ejsval
STRING_TO_EJSVAL_IMPL(EJSPrimString *str)
{
    ejsval l;
    uint64_t strBits = (uint64_t)str;
    EJS_ASSERT(str);
    EJS_ASSERT((strBits >> EJSVAL_TAG_SHIFT) == 0);
    l.asInt64 = strBits | EJSVAL_SHIFTED_TAG_STRING;
    return l;
}

static EJS_ALWAYS_INLINE EJSPrimString *
EJSVAL_TO_STRING_IMPL(ejsval l)
{
    return (EJSPrimString *)(l.asInt64 & EJSVAL_PAYLOAD_MASK);
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_CLOSUREENV_IMPL(ejsval l)
{
    return (uint32_t)(l.asInt64 >> EJSVAL_TAG_SHIFT) == EJSVAL_TAG_CLOSUREENV;
}

static EJS_ALWAYS_INLINE ejsval
CLOSUREENV_TO_EJSVAL_IMPL(EJSClosureEnv *env)
{
    ejsval l;
    uint64_t envBits = (uint64_t)env;
    EJS_ASSERT(env);
    EJS_ASSERT((envBits >> EJSVAL_TAG_SHIFT) == 0);
    l.asInt64 = envBits | EJSVAL_SHIFTED_TAG_CLOSUREENV;
    return l;
}

static EJS_ALWAYS_INLINE EJSClosureEnv *
EJSVAL_TO_CLOSUREENV_IMPL(ejsval l)
{
    return (EJSClosureEnv *)(l.asInt64 & EJSVAL_PAYLOAD_MASK);
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_BOOLEAN_IMPL(ejsval l)
{
    return (uint32_t)(l.asInt64 >> EJSVAL_TAG_SHIFT) == EJSVAL_TAG_BOOLEAN;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_TO_BOOLEAN_IMPL(ejsval l)
{
    return (EJSBool)l.asInt64;
}

static EJS_ALWAYS_INLINE ejsval
BOOLEAN_TO_EJSVAL_IMPL(EJSBool b)
{
    ejsval l;
    EJS_ASSERT(b == EJS_TRUE || b == EJS_FALSE);
    l.asInt64 = ((uint64_t)(uint32_t)b) | EJSVAL_SHIFTED_TAG_BOOLEAN;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_MAGIC_IMPL(ejsval l)
{
    return (l.asInt64 >> EJSVAL_TAG_SHIFT) == EJSVAL_TAG_MAGIC;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_PRIMITIVE_IMPL(ejsval l)
{
    return l.asInt64 < EJSVAL_SHIFTED_TAG_OBJECT;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_OBJECT_IMPL(ejsval l)
{
    EJS_ASSERT((l.asInt64 >> EJSVAL_TAG_SHIFT) <= EJSVAL_SHIFTED_TAG_OBJECT);
    return l.asInt64 >= EJSVAL_SHIFTED_TAG_OBJECT;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_OBJECT_OR_NULL_IMPL(ejsval l)
{
    EJS_ASSERT((l.asInt64 >> EJSVAL_TAG_SHIFT) <= EJSVAL_TAG_OBJECT);
    return l.asInt64 >= EJSVAL_SHIFTED_TAG_NULL;
}

static EJS_ALWAYS_INLINE EJSObject *
EJSVAL_TO_OBJECT_IMPL(ejsval l)
{
    uint64_t ptrBits = l.asInt64 & EJSVAL_PAYLOAD_MASK;
    //EJS_ASSERT((ptrBits & 0x7) == 0);
    return (EJSObject *)ptrBits;
}

static EJS_ALWAYS_INLINE ejsval
OBJECT_TO_EJSVAL_IMPL(EJSObject *obj)
{
    ejsval l;
    uint64_t objBits = (uint64_t)obj;
    EJS_ASSERT(obj);
    EJS_ASSERT((objBits >> EJSVAL_TAG_SHIFT) == 0);
    l.asInt64 = objBits | EJSVAL_SHIFTED_TAG_OBJECT;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_NULL_IMPL(ejsval l)
{
    return l.asInt64 == EJSVAL_SHIFTED_TAG_NULL;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_GCTHING_IMPL(ejsval l)
{
    return l.asInt64 >= EJSVAL_SHIFTED_TAG_STRING
;
}

static EJS_ALWAYS_INLINE void *
EJSVAL_TO_GCTHING_IMPL(ejsval l)
{
    uint64_t ptrBits = l.asInt64 & EJSVAL_PAYLOAD_MASK;
    //EJS_ASSERT((ptrBits & 0x7) == 0);
    return (void *)ptrBits;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_TRACEABLE_IMPL(ejsval l)
{
    return EJSVAL_IS_GCTHING_IMPL(l) && !EJSVAL_IS_NULL_IMPL(l);
}

static EJS_ALWAYS_INLINE uint32_t
EJSVAL_TRACE_KIND_IMPL(ejsval l)
{
    return (uint32_t)(EJSBool)!(EJSVAL_IS_OBJECT_IMPL(l));
}

static EJS_ALWAYS_INLINE ejsval
PRIVATE_PTR_TO_EJSVAL_IMPL(const void *ptr)
{
    ejsval l;
    uint64_t ptrBits = (uint64_t)ptr;
    EJS_ASSERT((ptrBits & 1) == 0);
    l.asInt64 = ptrBits >> 1;
    EJS_ASSERT(EJSVAL_IS_DOUBLE_IMPL(l));
    return l;
}

static EJS_ALWAYS_INLINE void *
EJSVAL_TO_PRIVATE_PTR_IMPL(ejsval l)
{
    EJS_ASSERT((l.asInt64 & 0x8000000000000000LL) == 0);
    return (void *)(l.asInt64 << 1);
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_SPECIFIC_INT32_IMPL(ejsval l, int32_t i32)
{
    return l.asInt64 == (((uint64_t)(uint32_t)i32) | EJSVAL_SHIFTED_TAG_INT32);
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_IS_SPECIFIC_BOOLEAN(ejsval l, EJSBool b)
{
    return l.asInt64 == (((uint64_t)(uint32_t)b) | EJSVAL_SHIFTED_TAG_BOOLEAN);
}

static EJS_ALWAYS_INLINE ejsval
MAGIC_TO_EJSVAL_IMPL(EJSWhyMagic why)
{
    ejsval l;
    l.asInt64 = ((uint64_t)(uint32_t)why) | EJSVAL_SHIFTED_TAG_MAGIC;
    return l;
}

static EJS_ALWAYS_INLINE EJSBool
EJSVAL_SAME_TYPE_IMPL(ejsval lhs, ejsval rhs)
{
    uint64_t lbits = lhs.asInt64, rbits = rhs.asInt64;
    return (lbits <= EJSVAL_SHIFTED_TAG_MAX_DOUBLE && rbits <= EJSVAL_SHIFTED_TAG_MAX_DOUBLE) ||
           (((lbits ^ rbits) & 0xFFFF800000000000LL) == 0);
}

static EJS_ALWAYS_INLINE ejsval
PRIVATE_UINT32_TO_EJSVAL_IMPL(uint32_t ui)
{
    ejsval l;
    l.asInt64 = (uint64_t)ui;
    EJS_ASSERT(EJSVAL_IS_DOUBLE_IMPL(l));
    return l;
}

static EJS_ALWAYS_INLINE uint32_t
EJSVAL_TO_PRIVATE_UINT32_IMPL(ejsval l)
{
    EJS_ASSERT((l.asInt64 >> 32) == 0);
    return (uint32_t)l.asInt64;
}

#endif  /* EJS_BITS_PER_WORD */

static EJS_ALWAYS_INLINE double
EJS_CANONICALIZE_NAN(double d)
{
    if (EJS_UNLIKELY(d != d)) {
        ejsval l;
        l.asInt64 = 0x7FF8000000000000LL;
        return l.asDouble;
    }
    return d;
}

#endif // _ejs_h_
