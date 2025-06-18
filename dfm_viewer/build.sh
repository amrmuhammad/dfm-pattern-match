#!/bin/bash

cd ~/dev/dfm_pattern_match4/dfm_viewer/build
rm -rf *

cmake ..
make -j$(nproc)
