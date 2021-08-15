#!/bin/bash
make && ~/bin/mkpsxiso -y config/3dcam.xml && pcsx-redux -run -iso 3dcam.cue
