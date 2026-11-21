#ifndef _MATH_H
#define _MATH_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* CactOS: freestanding x87 math (src/math.c).  No libm — the transcendentals
 * are FPU instructions and the integral helpers are bit/counting code. */

#define HUGE_VAL  (__builtin_huge_val())
#define HUGE_VALF (__builtin_huge_valf())
#define HUGE_VALL (__builtin_huge_vall())
#define INFINITY (__builtin_inff())
#define NAN      (__builtin_nanf(""))

#define isnan(x)     __builtin_isnan(x)
#define isinf(x)     __builtin_isinf(x)
#define isfinite(x)  __builtin_isfinite(x)

/* C99 7.12.3: what fpclassify() returns, and 7.12.4: the classification and
 * comparison macros.  Function-like macros, as the standard specifies, over
 * clang's builtins; the FP_* values are glibc's.  A C++ standard library header
 * reads FP_* directly (libc++'s std::fpclassify is written in terms of
 * __builtin_fpclassify and these names). */
#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

/* C99 7.12: the values ilogb()/logb() return for zero and NaN.  glibc's x86_64
 * values (both INT_MIN); llvm-libc's FPUtil reads them directly, so a ported
 * program (LLVM) needs them present. */
#define FP_ILOGB0   (-2147483647 - 1)
#define FP_ILOGBNAN (-2147483647 - 1)

#define fpclassify(x) __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, x)
#define signbit(x)    __builtin_signbit(x)
#define isnormal(x)   (fpclassify(x) == FP_NORMAL)

#define isgreater(x, y)      __builtin_isgreater(x, y)
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
#define isless(x, y)         __builtin_isless(x, y)
#define islessequal(x, y)    __builtin_islessequal(x, y)
#define islessgreater(x, y)  __builtin_islessgreater(x, y)
#define isunordered(x, y)    __builtin_isunordered(x, y)

/* C99 7.12: the widest floating types used by the math functions above. */
typedef float  float_t;
typedef double double_t;

double sin(double x);
double cos(double x);
double tan(double x);
double sqrt(double x);
double floor(double x);
double ceil(double x);
double round(double x);
double trunc(double x);
double fabs(double x);
double log(double x);
double log10(double x);
double exp(double x);
double pow(double x, double y);
double atan2(double y, double x);
double atan(double x);
double asin(double x);
double acos(double x);
double fmod(double x, double y);
double modf(double x, double *iptr);
double frexp(double x, int *exp);
double ldexp(double x, int exp);

/* The rest of POSIX 7.12 the ports name.  log2 and log1p are x87 instructions
 * (fyl2x / fyl2xp1); logb and rint are exponent/rounding structure; sinh/cosh/
 * tanh go through an accurate exp(x)-1; erf uses a Taylor series for small |x|
 * and the erfc continued fraction beyond. */
double log2(double x);
double log1p(double x);
double logb(double x);
double rint(double x);
double sinh(double x);
double cosh(double x);
double tanh(double x);
double erf(double x);

/* The rest of C99's 7.12 that the ports name: min/max (with the standard's NaN
 * rule — the non-NaN operand wins), the round-to-integer forms (lrint/llrint
 * round per the current mode, lround/llround half away from zero), and the
 * remaining elementary functions. */
double fmax(double x, double y);
double fmin(double x, double y);
long lrint(double x);
long long llrint(double x);
long lround(double x);
long long llround(double x);
double nearbyint(double x);
double copysign(double x, double y);
double nextafter(double x, double y);
double fdim(double x, double y);
double fma(double x, double y, double z);
double hypot(double x, double y);
double cbrt(double x);
double exp2(double x);
double expm1(double x);
double asinh(double x);
double acosh(double x);
double atanh(double x);
double remainder(double x, double y);
double scalbn(double x, int n);

/* Single-precision entry points — the full `*f` set a ported program names
 * (Mesa's u_math/u_format use fabsf, fmax, rintf, roundf, powf, lrintf …).
 * Each is the double version above, narrowed: exact for the integral ones and
 * correctly rounded for the rest, since double carries more than float's 24
 * bits. */
float fabsf(float x);
float floorf(float x);
float ceilf(float x);
float truncf(float x);
float roundf(float x);
float rintf(float x);
float nearbyintf(float x);
float fmodf(float x, float y);
float fmaxf(float x, float y);
float fminf(float x, float y);
float copysignf(float x, float y);
float nextafterf(float x, float y);
float fdimf(float x, float y);
float fmaf(float x, float y, float z);
float hypotf(float x, float y);
float cbrtf(float x);
float sqrtf(float x);
float sinf(float x);
float cosf(float x);
float tanf(float x);
float asinf(float x);
float acosf(float x);
float atanf(float x);
float atan2f(float y, float x);
float expf(float x);
float exp2f(float x);
float expm1f(float x);
float logf(float x);
float log10f(float x);
float log2f(float x);
float log1pf(float x);
float powf(float x, float y);
float sinhf(float x);
float coshf(float x);
float tanhf(float x);
float asinhf(float x);
float acoshf(float x);
float atanhf(float x);
float erff(float x);
long lrintf(float x);
long long llrintf(float x);
long lroundf(float x);
long long llroundf(float x);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
