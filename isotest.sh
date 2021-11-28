#!/bin/bash
make && ~/bin/mkpsxiso -y config/3dcam.xml && prime-run pcsx-redux -run -iso 3dcam.cue
