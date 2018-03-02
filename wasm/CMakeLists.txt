cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(GCC_COVERAGE_COMPILE_FLAGS "-DWASM_PLATFORM")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

include_directories(../ViroRenderer
                    
                    #../libs/freetype/include
                    #../libs/protobuf/include
                    #../libs/bullet/include
		    )

add_library( viro_renderer
             STATIC
	     
	     # Math
	     ../ViroRenderer/VROLineSegment.cpp
             ../ViroRenderer/VROTriangle.cpp
             ../ViroRenderer/VROQuaternion.cpp
             ../ViroRenderer/VROPlane.cpp
             ../ViroRenderer/VROFrustum.cpp
             ../ViroRenderer/VROFrustumPlane.cpp
             ../ViroRenderer/VROBoundingBox.cpp
             ../ViroRenderer/VROFrustumBoxIntersectionMetadata.cpp
             ../ViroRenderer/VROMatrix4f.cpp
             ../ViroRenderer/VROMath.cpp
             ../ViroRenderer/VROProjector.cpp
             ../ViroRenderer/VRODualQuaternion.cpp
	     ../ViroRenderer/VROVector3f.cpp
             ../ViroRenderer/VROVector4f.cpp)     

add_executable( viro_test
	        hello.cpp)

set_target_properties( viro_test PROPERTIES LINK_FLAGS "-s WASM=1 --emrun")
target_link_libraries ( viro_test viro_renderer )