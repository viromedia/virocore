#!/bin/bash

echo Building ViroRenderer
cmake -H. -Bproducts -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake
make --directory products
