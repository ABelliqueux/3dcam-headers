TARGET = main
TYPE = ps-exe

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
