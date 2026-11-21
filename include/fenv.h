#ifndef _FENV_H
#define _FENV_H

/* C linkage for the C++ consumers (libc++, LLVM, Mesa) that include this
 * header; a mangled reference would match no CactLibc export. */
#ifdef __cplusplus
extern "C" {
#endif

/* C99 7.6 <fenv.h>.
 *
 * x86_64 keeps the floating-point environment in two places: the x87 control
 * and status words, and the SSE MXCSR.  The compiler generates SSE for scalar
 * float and double, so MXCSR is what the rounding mode and the exception flags
 * actually are; glibc reports MXCSR too, and the x87 environment is carried
 * along so fesetenv()/feholdexcept() restore both.  The scheduler switches FPU
 * state lazily with FXSAVE/FXRSTOR, which saves MXCSR and the x87 words, so a
 * thread's environment is its own.
 *
 * The FE_* values are glibc's: FE_ALL_EXCEPT's bits are the six IEEE-754
 * exception bits and FE_TONEAREST..FE_TOWARDZERO are the x87 control-word
 * rounding codes (the MXCSR field is the same code shifted left by three). */

typedef struct {
    unsigned int  mxcsr;     /* SSE control and status word          */
    unsigned char x87env[28];/* what fnstenv stores: cw, sw, tw, ... */
} fenv_t;

typedef unsigned short fexcept_t;

#define FE_INVALID    0x01
#define FE_DENORMAL   0x02      /* x87 only; not one of the six exceptions */
#define FE_DIVBYZERO  0x04
#define FE_OVERFLOW   0x08
#define FE_UNDERFLOW  0x10
#define FE_INEXACT    0x20
#define FE_ALL_EXCEPT (FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | \
                       FE_OVERFLOW | FE_INVALID)

#define FE_TONEAREST  0x0000
#define FE_DOWNWARD   0x0400
#define FE_UPWARD     0x0800
#define FE_TOWARDZERO 0x0C00

/* The default environment, as if the FPU had just been reset. */
#define FE_DFL_ENV ((const fenv_t *)-1)

int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexcept(int excepts);

int fegetround(void);
int fesetround(int rounding_mode);

int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
