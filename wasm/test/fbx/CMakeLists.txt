cmake_minimum_required(VERSION 3.4.1)

SET(CMAKE_EXECUTABLE_SUFFIX ".html")
SET(TEST_GCC_COVERAGE_COMPILE_FLAGS "-DWASM_PLATFORM")
add_definitions(${TEST_GCC_COVERAGE_COMPILE_FLAGS})

add_subdirectory(../../ ../../products)

include_directories(../../src/cpp)
add_executable( viro_test
	        bootstrap.cpp)

set_target_properties( viro_test PROPERTIES LINK_FLAGS "-s WASM=1 -s USE_ZLIB=1 -s USE_FREETYPE=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_SOURCE_DIR}/../products/preload@/ --shell-file ${CMAKE_SOURCE_DIR}/../viro_shell.html --emrun")

target_link_libraries ( viro_test 
                        viro_renderer protobuf-lite
                        jpeg
                        bullet_linearmath
                        bullet_softbody
                        bullet_dynamics
                        bullet_collision )