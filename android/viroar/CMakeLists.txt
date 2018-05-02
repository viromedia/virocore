cmake_minimum_required(VERSION 3.4.1)

SET(VIRO_ANDROID_INCLUDES ${CMAKE_SOURCE_DIR}/../libs)
SET(VIRO_RENDERER_SRC     ${CMAKE_SOURCE_DIR}/../../ViroRenderer)
SET(VIRO_ANDROID_SRC      ${CMAKE_SOURCE_DIR}/../app/src/main/cpp)
SET(VIRO_AR_LIBS          ${CMAKE_SOURCE_DIR}/src/main/jniLibs)
SET(VIRO_AR_SRC           ${CMAKE_SOURCE_DIR}/src/main/cpp)

INCLUDE_DIRECTORIES(${VIRO_RENDERER_SRC}
                    ${VIRO_ANDROID_SRC}
                    ${VIRO_ANDROID_SRC}/arcore
                    ${VIRO_ANDROID_INCLUDES}/arcore/include)

ADD_LIBRARY(viro_arcore
            SHARED

            ${VIRO_AR_SRC}/arcore/ARCore_Native.cpp
            ${VIRO_AR_SRC}/arcore/VRORendererARCoreNative_JNI.cpp
           )

# Add pre-built libraries
ADD_LIBRARY(lib-arcore SHARED IMPORTED )
SET_TARGET_PROPERTIES(lib-arcore
                      PROPERTIES IMPORTED_LOCATION
                      ${VIRO_AR_LIBS}/${ANDROID_ABI}/libarcore_sdk_c.so )

FIND_LIBRARY(lib-mediandk mediandk)

TARGET_LINK_LIBRARIES(# Specifies the target library.
                      viro_arcore

                      # Libraries linked with target
                      lib-arcore
                      ${lib-mediandk}
                      )