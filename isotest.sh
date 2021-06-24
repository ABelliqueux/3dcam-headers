#!/bin/bash

make && ~/bin/mkpsxiso -y config/OverlayExample.xml && pcsx-redux -run -iso OverlayExample.cue
