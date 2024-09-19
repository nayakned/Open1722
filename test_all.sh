#!/bin/bash
set -ev

rm -rf build
mkdir build
cd build
cmake .. -DUNIT_TESTING=on
make -j`nproc`
make test
