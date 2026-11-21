#include "fenv.h"

/* <fenv.h> over x86_64's two control words.  The x87 environment moves as a
 * 28-byte block (fnstenv/fldenv); MXCSR moves on its own (stmxcsr/ldmxcsr).
 * fnstenv has a side effect the manual warns about: it masks every exception in
 * the control word it stores *and* leaves the masked control word in the FPU,
 * so every user has to put the control word back afterwards (x87_restore_cw). */

#include "stdint.h"
#include "string.h"

#define MXCSR_EXC_MASK 0x3Fu    /* the six sticky exception flags */
#define MXCSR_RC_MASK  0x6000u  /* rounding control, bits 13-14 */
#define X87_CW_EXC_MASK 0x3Fu
#define X87_CW_RC_MASK  0xC00u  /* rounding control, bits 10-11 */
#define X87_CW_DEFAULT  0x037Fu
#define MXCSR_DEFAULT   0x1F80u

static unsigned short x87_cw_get(void) {
    unsigned short cw;
    __asm__ volatile("fnstcw %0" : "=m"(cw));
    return cw;
}

static void x87_cw_set(unsigned short cw) {
    __asm__ volatile("fldcw %0" : : "m"(cw));
}

static unsigned int mxcsr_get(void) {
    unsigned int m;
    __asm__ volatile("stmxcsr %0" : "=m"(m));
    return m;
}

static void mxcsr_set(unsigned int m) {
    __asm__ volatile("ldmxcsr %0" : : "m"(m));
}

static void x87_env_store(unsigned char *env) {
    unsigned char tmp[28];
    __asm__ volatile("fnstenv %0" : "=m"(tmp));
    x87_cw_set(x87_cw_get());   /* undo fnstenv's masking, keep the real cw */
    memcpy(env, tmp, sizeof(tmp));
}

static void x87_env_load(const unsigned char *env) {
    unsigned char tmp[28];
    memcpy(tmp, env, sizeof(tmp));
    __asm__ volatile("fldenv %0" : : "m"(tmp));
}

static void x87_clear_exceptions(void) {
    __asm__ volatile("fnclex");
}

/* The mxcsr field of a saved environment is stored with its rounding mode, but
 * an environment is also the place the *exception flags* live: glibc keeps them
 * here the same way, so fesetenv() restores flags too. */

int feclearexcept(int excepts) {
    unsigned int m = mxcsr_get() & ~((unsigned int)excepts & MXCSR_EXC_MASK);
    mxcsr_set(m);
    if (excepts & (FE_INVALID | FE_DENORMAL))
        x87_clear_exceptions();
    return 0;
}

int fetestexcept(int excepts) {
    unsigned int raised = mxcsr_get() & MXCSR_EXC_MASK;
    return (int)(raised & (unsigned int)excepts);
}

int fegetexceptflag(fexcept_t *flagp, int excepts) {
    if (!flagp) return -1;
    *flagp = (fexcept_t)(mxcsr_get() & (unsigned int)excepts & MXCSR_EXC_MASK);
    return 0;
}

int fesetexceptflag(const fexcept_t *flagp, int excepts) {
    unsigned int m;
    if (!flagp) return -1;
    m = mxcsr_get();
    m = (m & ~((unsigned int)excepts & MXCSR_EXC_MASK)) |
        (*flagp & (unsigned int)excepts & MXCSR_EXC_MASK);
    mxcsr_set(m);
    return 0;
}

int feraiseexcept(int excepts) {
    /* Setting a sticky status bit is all "raising" means for the FPU: the
     * handler, if any, is the kernel's business, and CactOS has none. */
    mxcsr_set(mxcsr_get() | ((unsigned int)excepts & MXCSR_EXC_MASK));
    return 0;
}

int fegetround(void) {
    return (int)((mxcsr_get() & MXCSR_RC_MASK) >> 3);
}

int fesetround(int rounding_mode) {
    if (rounding_mode != FE_TONEAREST && rounding_mode != FE_DOWNWARD &&
        rounding_mode != FE_UPWARD && rounding_mode != FE_TOWARDZERO)
        return -1;
    mxcsr_set((mxcsr_get() & ~MXCSR_RC_MASK) | ((unsigned int)rounding_mode << 3));
    x87_cw_set((unsigned short)((x87_cw_get() & ~X87_CW_RC_MASK) | (unsigned int)rounding_mode));
    return 0;
}

int fegetenv(fenv_t *envp) {
    if (!envp) return -1;
    x87_env_store(envp->x87env);
    envp->mxcsr = mxcsr_get();
    return 0;
}

int feholdexcept(fenv_t *envp) {
    if (!envp) return -1;
    fegetenv(envp);
    /* Non-stop: clear the flags and mask the exceptions in both words. */
    mxcsr_set(mxcsr_get() & ~MXCSR_EXC_MASK);
    x87_clear_exceptions();
    x87_cw_set((unsigned short)(x87_cw_get() | X87_CW_EXC_MASK));
    return 0;
}

int fesetenv(const fenv_t *envp) {
    if (!envp) return -1;
    if (envp == FE_DFL_ENV) {
        x87_cw_set(X87_CW_DEFAULT);
        mxcsr_set(MXCSR_DEFAULT);
        return 0;
    }
    x87_env_load(envp->x87env);
    /* fldenv restores the stored control word, which x87_env_store() kept real;
     * the exception flags come from the status word that was stored. */
    mxcsr_set(envp->mxcsr);
    return 0;
}

int feupdateenv(const fenv_t *envp) {
    unsigned int raised = mxcsr_get() & MXCSR_EXC_MASK;
    int r = fesetenv(envp);
    /* The environment is restored first, then the exceptions that were raised
     * inside it are raised again — C99 7.6.4.3. */
    mxcsr_set(mxcsr_get() | raised);
    return r;
}
