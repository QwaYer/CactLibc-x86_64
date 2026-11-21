#ifndef _MATH_H
#define _MATH_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* CactOS: freestanding x87 math (src/math.c).  No libm — the transcendentals
 * are FPU instructions and the integral helpers are bit/counting code. */

#define HUGE_VAL (__builtin_huge_val())
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

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
