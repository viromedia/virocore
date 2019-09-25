#!/bin/bash

echo Building ViroRenderer [target: $1]
cmake -H. -Bproducts -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake
cmake --build products --target viro_fbx_test -- -j 4
