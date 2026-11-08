/*
 * Linker script for CactOS userspace ld.so.
 *
 * Fixed base 0x18000000: above the main PIE images (0x08000000) and libc
 * (0x10000000), and inside the user half of the address space
 * ([USER_SPACE_START, KERNEL_BASE) = [0x08000000, 0xC0000000)) but below the
 * mmap window (MMAP_BASE = 0x80000000).  The kernel maps PT_INTERP
 * interpreters exactly at their link-time vaddrs (bias 0), and because every
 * image here is fixed-address, the same holds for the program and libc: three
 * disjoint, low addresses keep ld.so's own data out of the way of the image it
 * relocates.
 */

ENTRY(_start)

SECTIONS {
    . = 0x18000000;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    . = ALIGN(0x1000);
    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    }
}
