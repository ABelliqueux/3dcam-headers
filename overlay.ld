__heap_base = MAX(__lvl0_end, __lvl1_end);

SECTIONS {            
        OVERLAY __bss_end : NOCROSSREFS SUBALIGN(4)
    {
        .lvlload
        {
            load_all_overlays_here = .;
        }
        .lvl0
        {
			KEEP(levels/level0.o(.text))
			__lvl0_ctor = .;
			KEEP(levels/level0.o(.text.startup._GLOBAL__*))
			KEEP(levels/level0.o(.text.*))
			KEEP(levels/level0.o(.rodata*))
            KEEP(levels/level0.o(.sdata*))
			KEEP(levels/level0.o(.data*))
            KEEP(levels/level0.o(.sbss*))
			KEEP(levels/level0.o(.bss*))
			KEEP(levels/level0.o(.ctors))
			
            /*KEEP(TIM/bg_camPath_*.o(.text))
			__lvl0_ctor = .;
			KEEP(TIM/bg_camPath_*.o(.text.startup._GLOBAL__*))
			KEEP(TIM/bg_camPath_*.o(.text.*))
			KEEP(TIM/bg_camPath_*.o(.rodata*))
            KEEP(TIM/bg_camPath_*.o(.sdata*))
			KEEP(TIM/bg_camPath_*.o(.data*))
            KEEP(TIM/bg_camPath_*.o(.sbss*))
			KEEP(TIM/bg_camPath_*.o(.bss*))
			KEEP(TIM/bg_camPath_*.o(.ctors))
			*/
			. = ALIGN(4);
			__lvl0_end = .;			
		}

        .lvl1
        {
			KEEP(levels/level1.o(.text))
			__lvl1_ctor = .;
			KEEP(levels/level1.o(.text.startup._GLOBAL__*))
			KEEP(levels/level1.o(.text.*))
			KEEP(levels/level1.o(.rodata*))
            KEEP(levels/level1.o(.sdata*))
			KEEP(levels/level1.o(.data*))
            KEEP(levels/level1.o(.sbss*))
			KEEP(levels/level1.o(.bss*))
			KEEP(levels/level1.o(.ctors))
			
			. = ALIGN(4);
			__lvl1_end = .;			
		}
	}
}
