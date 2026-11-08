/*
 * dlfcn host-test fixture: a second x86_64 plugin image, used only to check that
 * dlmopen() puts an object in its *own* namespace — its symbols must be reachable
 * through the handle dlmopen() returns and through no other scope, so the symbol
 * names here are unique to this file.
 *
 * Nothing here is executed; the test maps it with the real loader and reads the
 * resulting memory.
 */

int plugin_iso_data = 0x1509;

int plugin_iso_marker(void) { return 1; }
