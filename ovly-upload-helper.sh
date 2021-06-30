#!/bin/bash

# Path to nops executable
NOPS="nops"

if [ $# -eq 0 ]
  then
    echo "PSX Overlay Upload helper script
    
Upload a binary and the corresponding executable to a real PSX memory, via unirom + serial cable.

This script is dependant on NOTpsxserial being available on your system : https://github.com/JonathanDotCel/NOTPSXSerial
Edit the $NOPS value to reflect the executable path on your system, e.g : 
    \$NOPS = '/blah/nops'
Usage : ./ovly_upload_helper.sh bin_filename psx_exe_name com_port
    
    - bin_filename , eg : Overlay.lvl0
    - psx_exe_name, e.g : main.ps-exe
    - com_port, e.g : /dev/ttyUSB0, COM1
"
else
# Find map file corresponding to ps-exe
MAP_FILE="`echo $2 | awk -F. '{print $1}'`.map"
# Find loading address
LOAD_ADDR="0x`cat $MAP_FILE | grep load_all_overlays_here | awk '{print $1}' | cut -c 11-`"

$NOPS /debug $3
$NOPS /fast /bin $LOAD_ADDR $1 $3
$NOPS /fast /exe $2 $3
$NOPS /slow $3
fi
