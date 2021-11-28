TARGET = 3dcam

.PHONY: all cleansub

all:
	mkpsxiso -y ./config/3dcam.xml

cleansub:
	$(MAKE) clean
	rm -f $(TARGET).cue $(TARGET).bin
	rm -f *.mcd *.frag *.lua *.vert
	
SRCS = src/main.c \
src/pad.c \
src/math.c \
src/camera.c \
src/physics.c \
src/graphics.c \
src/psx.c \
src/space.c \
src/sound.c \
levels/level0.c \
levels/level1.c \
TIM/home.tim \
TIM/cat.tim \
TIM/lara.tim \
TIM/bg_camPath.tim \
TIM/bg_camPath_001.tim \
TIM/bg_camPath_002.tim \
TIM/bg_camPath_003.tim \
TIM/bg_camPath_004.tim \
TIM/bg_camPath_005.tim \
TIM/ground.tim \
TIM/trees.tim \
TIM/woods.tim \
VAG/hello.vag \
VAG/poly.vag \
VAG/0_come.vag \
VAG/1_cuek.vag \
VAG/2_erro.vag \
VAG/3_hehe.vag \
VAG/4_m4a1.vag \
VAG/5_punc.vag \
VAG/7_wron.vag \
VAG/8_yooo.vag \

OVERLAYSCRIPT  ?= overlay.ld
OVERLAYSECTION ?= .lvl0 .lvl1

include ./common.mk 
