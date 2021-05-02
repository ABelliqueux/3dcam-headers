#!/bin/bash

make && mkpsxiso -y config/OverlayExample.xml && prime-run pcsx-redux -run -iso OverlayExample.cue
