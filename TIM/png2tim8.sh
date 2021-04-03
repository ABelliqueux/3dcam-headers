#!/bin/bash

# convert to 256 colors
#~ for i in bg_*.png; do convert $i -colors 256 $i; done

# convert to tim
for i in bg_*.png; do img2tim -t -bpp 8 -org 320 0 -plt 0 481 -o ${i%.*}.tim $i;done


# Other PNGs


img2tim -org 576 256 -plt 0 480 -bpp 8-o cat.tim cat.png

img2tim -usealpha -org 576 0 -plt 0 481 -bpp 8 -o home.tim home.png

img2tim -org 320 256 -plt 0 482 -bpp 8 -o lara.tim lara.png
