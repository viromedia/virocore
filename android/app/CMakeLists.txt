# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

include_directories(../../ViroRenderer
                    src/main/cpp
                    ../libs/gvr/include
                    ../libs/freetype/include)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.

             # Android Platform
             src/main/cpp/native-lib.cpp
             src/main/cpp/debug_stacktrace.cpp
             src/main/cpp/debug_mapinfo.cpp
             src/main/cpp/VROLooper.cpp
             src/main/cpp/VROSceneRendererCardboard.cpp
             src/main/cpp/VROSample.cpp
             src/main/cpp/VROImageAndroid.cpp
             src/main/cpp/VROPCMAudioPlayer.cpp
             src/main/cpp/VROVideoTextureAndroid.cpp
             src/main/cpp/VROSoundEffectAndroid.cpp
             src/main/cpp/VROAudioPlayerAndroid.cpp
             src/main/cpp/VROAVPlayer.cpp
             src/main/cpp/VROVideoTextureAVP.cpp
             src/main/cpp/VROTypefaceAndroid.cpp

             # Android JNI
             src/main/cpp/jni/VRORenderer_JNI.cpp
             src/main/cpp/jni/Node_JNI.cpp
             src/main/cpp/jni/Scene_JNI.cpp
             src/main/cpp/jni/Box_JNI.cpp
             src/main/cpp/jni/Surface_JNI.cpp
             src/main/cpp/jni/Sphere_JNI.cpp
             src/main/cpp/jni/Material_JNI.cpp
             src/main/cpp/jni/Image_JNI.cpp
             src/main/cpp/jni/Texture_JNI.cpp
             src/main/cpp/jni/VideoTexture_JNI.cpp
             src/main/cpp/jni/VideoDelegate_JNI.cpp
             src/main/cpp/jni/RenderContext_JNI.cpp

             # Math
             ../../ViroRenderer/VROLineSegment.cpp
             ../../ViroRenderer/VROTriangle.cpp
             ../../ViroRenderer/VROQuaternion.cpp
             ../../ViroRenderer/VROPlane.cpp
             ../../ViroRenderer/VROFrustum.cpp
             ../../ViroRenderer/VROFrustumPlane.cpp
             ../../ViroRenderer/VROBoundingBox.cpp
             ../../ViroRenderer/VROVector3f.cpp
             ../../ViroRenderer/VROVector4f.cpp
             ../../ViroRenderer/VROMatrix4f.cpp
             ../../ViroRenderer/VROMath.cpp
             ../../ViroRenderer/VROProjector.cpp

             # Util
             ../../ViroRenderer/VROAllocationTracker.cpp
             ../../ViroRenderer/VROTime.cpp
             ../../ViroRenderer/VROLog.cpp
             ../../ViroRenderer/VROByteBuffer.cpp
             ../../ViroRenderer/VROImageUtil.cpp
             ../../ViroRenderer/VROData.cpp
             ../../ViroRenderer/VROGeometryUtil.cpp
             ../../ViroRenderer/VROTextureUtil.cpp
             ../../ViroRenderer/VROStringUtil.cpp
             ../../ViroRenderer/VROPlatformUtil.cpp
             ../../ViroRenderer/VROOBJLoader.cpp
             ../../ViroRenderer/tiny_obj_loader.cc

             # Renderer
             ../../ViroRenderer/VROScene.cpp
             ../../ViroRenderer/VROSceneControllerInternal.cpp
             ../../ViroRenderer/VROCamera.cpp
             ../../ViroRenderer/VRONode.cpp
             ../../ViroRenderer/VROGeometry.cpp
             ../../ViroRenderer/VROGeometryElement.cpp
             ../../ViroRenderer/VROGeometrySource.cpp
             ../../ViroRenderer/VROMaterial.cpp
             ../../ViroRenderer/VROMaterialVisual.cpp
             ../../ViroRenderer/VROTexture.cpp
             ../../ViroRenderer/VROLight.cpp
             ../../ViroRenderer/VROHoverController.cpp
             ../../ViroRenderer/VROBillboardConstraint.cpp
             ../../ViroRenderer/VROTransformConstraint.cpp
             ../../ViroRenderer/VRORenderer.cpp
             ../../ViroRenderer/VROFrameSynchronizerInternal.cpp
             ../../ViroRenderer/VROGeometrySubstrate.cpp
             ../../ViroRenderer/VROTextureSubstrate.cpp
             ../../ViroRenderer/VROMaterialSubstrate.cpp
             ../../ViroRenderer/VROCameraMutable.cpp

             # Animation
             ../../ViroRenderer/VROTimingFunction.cpp
             ../../ViroRenderer/VROTransaction.cpp
             ../../ViroRenderer/VROAnimatable.cpp
             ../../ViroRenderer/VROAction.cpp

             # Shapes
             ../../ViroRenderer/VROBox.cpp
             ../../ViroRenderer/VROSkybox.cpp
             ../../ViroRenderer/VROSurface.cpp
             ../../ViroRenderer/VROTorusKnot.cpp
             ../../ViroRenderer/VROShapeUtils.cpp
             ../../ViroRenderer/VROSphere.cpp
             ../../ViroRenderer/VROPolyline.cpp
             ../../ViroRenderer/VROVideoSurface.cpp

             # UI
             ../../ViroRenderer/VROReticle.cpp
             ../../ViroRenderer/VROText.cpp
             ../../ViroRenderer/VROKnuthPlassFormatter.cpp

             # OpenGL
             ../../ViroRenderer/VROGeometrySubstrateOpenGL.cpp
             ../../ViroRenderer/VROMaterialSubstrateOpenGL.cpp
             ../../ViroRenderer/VROTextureSubstrateOpenGL.cpp
             ../../ViroRenderer/VROUniform.cpp
             ../../ViroRenderer/VROShaderProgram.cpp
             ../../ViroRenderer/VROShaderModifier.cpp
             ../../ViroRenderer/VROLightingUBO.cpp
             ../../ViroRenderer/VROGlyphOpenGL.cpp
             )

# Add pre-built libraries
add_library( lib-gvr
             SHARED
             IMPORTED )
set_target_properties( lib-gvr
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libgvr.so )

add_library( lib-gvr-audio
             SHARED
             IMPORTED )
set_target_properties( lib-gvr-audio
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libgvr_audio.so )

add_library( lib-freetype
             SHARED
             IMPORTED )
set_target_properties( lib-freetype
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libfreetype.so )


# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build. The first argument to find_library is the output
# variable where the path to the library will be stored.

find_library( lib-log log )
find_library( lib-android android )
find_library( lib-jnigraphics jnigraphics )
find_library( lib-mediandk mediandk )
find_library( lib-openmax OpenMAXAL )
find_library( lib-opensles OpenSLES )
find_library( lib-egl EGL )
find_library( lib-GLESv3 GLESv3 )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${lib-log}
                       ${lib-android}
                       ${lib-jnigraphics}
                       ${lib-mediandk}
                       ${lib-openmax}
                       ${lib-opensles}
                       ${lib-egl}
                       ${lib-GLESv3}
                       lib-gvr
                       lib-gvr-audio
                       lib-freetype)
