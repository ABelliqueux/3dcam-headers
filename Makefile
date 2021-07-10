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

# img2tim -t -bpp 8 -org 320 0 -plt 0 481 -o bg.tim bg.png
include ./common.mk 
include ./thirdparty/nugget/common.mk \
