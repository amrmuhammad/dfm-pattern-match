#!/bin/bash

cd ./dfm-pattern-capture-enhanced5/build
rm -rf *
cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG=ON ..
make
