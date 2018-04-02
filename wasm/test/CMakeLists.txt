cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(TEST_GCC_COVERAGE_COMPILE_FLAGS "-DWASM_PLATFORM")
add_definitions(${TEST_GCC_COVERAGE_COMPILE_FLAGS})

message( STATUS "VIRO TEST DIR:           " ${VIRO_TEST_DIR} )

# Build viro_renderer into its own products dir (note that the binary
# directory here is relative to *this* file's binary directory
add_subdirectory(../ ../../products)

# Build the executable
include_directories(../src/cpp ../../ViroRenderer)
add_executable( viro_test
	        ${VIRO_TEST_DIR}/bootstrap.cpp)

set_target_properties( viro_test PROPERTIES LINK_FLAGS "-s WASM=1 -s USE_ZLIB=1 -s USE_FREETYPE=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_SOURCE_DIR}/products/preload@/ --shell-file ${CMAKE_SOURCE_DIR}/viro_shell.html --emrun")

target_link_libraries ( viro_test 
                        viro_renderer protobuf-lite
                        jpeg
                        bullet_linearmath
                        bullet_softbody
                        bullet_dynamics
                        bullet_collision )