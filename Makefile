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
../common/crt0/crt0.s \
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

CPPFLAGS += -I../psyq/include
LDFLAGS += -L../psyq/lib
LDFLAGS += -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgs
LDFLAGS += -lgte
LDFLAGS += -lgun
LDFLAGS += -lhmd
LDFLAGS += -lmath
LDFLAGS += -lmcrd
LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
LDFLAGS += -ltap
LDFLAGS += -lcd
LDFLAGS += -Wl,--end-group

include ../common.mk \
