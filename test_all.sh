#!/bin/bash
set -ev

rm -rf build
mkdir build
cd build
cmake ..
make -j`nproc`
make unittests
make test

lcov --capture --directory . --output-file main_coverage.info --include '*/include/*/*.h' --exclude '*/unit/test-*.c'
genhtml main_coverage.info -o ./coverage/
