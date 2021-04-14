__heap_base = MAX(__ovly0_end, __ovly1_end);

SECTIONS {			
	OVERLAY __bss_end : NOCROSSREFS SUBALIGN(4)
	{
		.ovly0
        {
			KEEP(levels/level.o(.text))
			__ovly0_ctor = .;
			KEEP(levels/level.o(.text.startup._GLOBAL__*))
			KEEP(levels/level.o(.text.*))
			KEEP(levels/level.o(.rodata*))
            KEEP(levels/level.o(.sdata*))
			KEEP(levels/level.o(.data*))
            KEEP(levels/level.o(.sbss*))
			KEEP(levels/level.o(.bss*))
			KEEP(levels/level.o(.ctors))
			
			. = ALIGN(4);
			__ovly0_end = .;			
		}

        .ovly1
        {
			KEEP(levels/level1.o(.text))
			__ovly1_ctor = .;
			KEEP(levels/level1.o(.text.startup._GLOBAL__*))
			KEEP(levels/level1.o(.text.*))
			KEEP(levels/level1.o(.rodata*))
            KEEP(levels/level1.o(.sdata*))
			KEEP(levels/level1.o(.data*))
            KEEP(levels/level1.o(.sbss*))
			KEEP(levels/level1.o(.bss*))
			KEEP(levels/level1.o(.ctors))
			
			. = ALIGN(4);
			__ovly1_end = .;			
		}
	}
}
