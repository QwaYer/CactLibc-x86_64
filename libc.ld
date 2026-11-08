/*
 * Linker script for the CactOS shared libc (clibc.so).
 *
 * Fixed base 0x10000000: the user half of the address space is
 * [0x08000000, 0xC0000000) — the main PIE images sit at 0x08000000 and ld.so at
 * 0x18000000 — and CactOS maps every image at its link-time address (bias 0),
 * so the base only has to be disjoint from the two neighbours and below the
 * mmap window (MMAP_BASE = 0x80000000).
 *
 * The `.rela.*` names are the x86-64 ones; the `.rel.*` pair is kept so a
 * relocatable built by an older toolchain still lands in a named section
 * rather than an orphan one.
 */

SECTIONS {
    . = 0x10000000;

    .dynamic   : { *(.dynamic) }
    .hash      : { *(.hash) }
    .gnu.hash  : { *(.gnu.hash) }
    .dynsym    : { *(.dynsym) }
    .dynstr    : { *(.dynstr) }
    .rela.dyn  : { *(.rela.dyn) }
    .rela.plt  : { *(.rela.plt) }
    .rel.dyn   : { *(.rel.dyn) }
    .rel.plt   : { *(.rel.plt) }

    .init      : { *(.init) }
    .plt       : { *(.plt) }
    .text      : { *(.text .text.*) }
    .fini      : { *(.fini) }

    .rodata    : { *(.rodata .rodata.*) }
    .eh_frame  : { *(.eh_frame) }

    . = ALIGN(0x1000);
    .data.rel.ro : { *(.data.rel.ro .data.rel.ro.*) }
    .data      : { *(.data .data.*) }
    .got       : { *(.got) }
    .got.plt   : { *(.got.plt) }
    .bss       : { *(.bss .bss.*) *(COMMON) }
}
