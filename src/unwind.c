/*
 * DWARF frame registration — __register_frame() / __deregister_frame().
 *
 * These are libgcc's (crtstuff + unwind-dw2-fde), called by a program that
 * hands the unwinder a table of FDEs it built at run time — LLVM's JIT does
 * exactly that for the code it emits.  CactOS builds its C++ with exceptions
 * and unwind tables off, so nothing ever walks those tables; the C library
 * provides the symbols so a ported program links, and the calls do nothing.
 * (Backtrace is off too, so nothing needs the frames registered.)
 */
void __register_frame(void *begin)   { (void)begin; }
void __deregister_frame(void *begin) { (void)begin; }
