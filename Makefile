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

OVERLAYSCRIPT  ?= overlay.ld
OVERLAYSECTION ?= .lvl0 .lvl1

include ./common.mk 
