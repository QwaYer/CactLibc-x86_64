/*
 * dlfcn host-test fixture: an x86_64 plugin whose `__thread` uses the DEFAULT
 * (general-dynamic) model.  GNU ld turns its access into a pair of GOT words
 * (R_X86_64_DTPMOD64 / _DTPOFF64 — the module id and the variable's offset)
 * plus a __tls_get_addr call, which is the path the loader has to fill from the
 * module it gives the plugin's TLS.
 */

__thread int plugin_gd_var = 1;

int plugin_gd_get(void) { return plugin_gd_var; }
