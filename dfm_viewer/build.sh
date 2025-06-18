#!/bin/bash

cd ./dfm_viewer/build
rm -rf *

cmake ..
make -j$(nproc)
