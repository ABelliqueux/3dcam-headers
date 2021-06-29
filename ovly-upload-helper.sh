#!/bin/bash
if [ $# -eq 0 ]
  then
    echo "PSX Overlay Upload helper script
    
Upload a binary and the corresponding executable to a real PSX memory, via unirom + serial cable.

This script is dependant on NOTpsxserial being available on your system : https://github.com/JonathanDotCel/NOTPSXSerial
Edit the $NOPS value to reflect the executable path on your system, e.g : 
    \$NOPS = '/blah/nops'
Usage : ./ovly_upload_helper.sh bin_load_adress bin_filename psx_exe_name com_port
    
    - bin_load_adress, eg : 0x80010000 (This should correspond to the 'load_all_overlays_here' adress in your .map file. )
    - bin_filename , eg : Overlay.lvl0
    - psx_exe_name, e.g : main.ps-exe
    - com_port, e.g : /dev/ttyUSB0, COM1
"
else
# Path to nops executable
NOPS="nops"
# $1 = bin loading address ( see .map's "load_all_overlays_here" address )
# $2 = bin file
# $3 = ps-exe file
# $4 = comport 
$NOPS /debug $4
$NOPS /fast /bin $1 $2 $4
$NOPS /fast /exe $3 $4
$NOPS /slow $4
fi
