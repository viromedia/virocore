#!/bin/bash

echo Building ViroRenderer [test: $1]
cmake -Hfbx -Bproducts -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake
make --directory products
