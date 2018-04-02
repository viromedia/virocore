#!/bin/bash

echo Creating preload directory
rm -rf products/preload && mkdir products/preload
cp -r $1/preload/* products/preload
cp -r ../preload/* products/preload

echo Building ViroRenderer [test: $1]
cmake -H. -Bproducts -DVIRO_TEST_DIR=$1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake
make --directory products
