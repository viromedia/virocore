# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

include_directories(../../ViroRenderer
                    src/main/cpp
                    ../libs/gvr/include
                    ../libs/ovr/include
                    ../libs/freetype/include
                    ../libs/protobuf/include
                    ../libs/bullet/include
                    ../libs/opencv/include)

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
             src/main/cpp/VROSceneRendererGVR.cpp
             src/main/cpp/VROSceneRendererOVR.cpp
             src/main/cpp/VROSample.cpp
             src/main/cpp/VROImageAndroid.cpp
             src/main/cpp/VROPCMAudioPlayer.cpp
             src/main/cpp/VROVideoTextureAndroid.cpp
             src/main/cpp/VROAudioPlayerAndroid.cpp
             src/main/cpp/VROAVPlayer.cpp
             src/main/cpp/VROVideoTextureAVP.cpp
             src/main/cpp/VROTypefaceAndroid.cpp
             src/main/cpp/VROInputControllerDaydream.cpp
             src/main/cpp/VROInputControllerCardboard.cpp
             src/main/cpp/VROInputControllerOVR.cpp

             # Android JNI
             src/main/cpp/jni/VRORenderer_JNI.cpp
             src/main/cpp/jni/Node_JNI.cpp
             src/main/cpp/jni/Scene_JNI.cpp
             src/main/cpp/jni/Camera_JNI.cpp
             src/main/cpp/jni/Box_JNI.cpp
             src/main/cpp/jni/Controller_JNI.cpp
             src/main/cpp/jni/Surface_JNI.cpp
             src/main/cpp/jni/Sphere_JNI.cpp
             src/main/cpp/jni/Material_JNI.cpp
             src/main/cpp/jni/Image_JNI.cpp
             src/main/cpp/jni/Texture_JNI.cpp
             src/main/cpp/jni/VideoTexture_JNI.cpp
             src/main/cpp/jni/VideoDelegate_JNI.cpp
             src/main/cpp/jni/RenderContext_JNI.cpp
             src/main/cpp/jni/EventDelegate_JNI.cpp
             src/main/cpp/jni/AnimationChain_JNI.cpp
             src/main/cpp/jni/AnimationGroup_JNI.cpp
             src/main/cpp/jni/ExecutableAnimation_JNI.cpp
             src/main/cpp/jni/AmbientLight_JNI.cpp
             src/main/cpp/jni/SpotLight_JNI.cpp
             src/main/cpp/jni/DirectionalLight_JNI.cpp
             src/main/cpp/jni/OmniLight_JNI.cpp
             src/main/cpp/jni/Sound_JNI.cpp
             src/main/cpp/jni/SoundField_JNI.cpp
             src/main/cpp/jni/SpatialSound_JNI.cpp
             src/main/cpp/jni/SoundDelegate_JNI.cpp
             src/main/cpp/jni/Text_JNI.cpp
             src/main/cpp/jni/TextDelegate_JNI.cpp
             src/main/cpp/jni/SoundData_JNI.cpp
             src/main/cpp/jni/Object_JNI.cpp
             src/main/cpp/jni/OBJLoaderDelegate_JNI.cpp
             src/main/cpp/jni/LazyMaterial_JNI.cpp
             src/main/cpp/jni/Polyline_JNI.cpp
             src/main/cpp/jni/OpenCV_JNI.cpp
             src/main/cpp/jni/ImageTracker_JNI.cpp
             src/main/cpp/jni/ImageTrackerOutput_JNI.cpp
             src/main/cpp/jni/PhysicsDelegate_JNI.cpp
             src/main/cpp/jni/TransformDelegate_JNI.cpp

             # Math
             ../../ViroRenderer/VROLineSegment.cpp
             ../../ViroRenderer/VROTriangle.cpp
             ../../ViroRenderer/VROQuaternion.cpp
             ../../ViroRenderer/VROPlane.cpp
             ../../ViroRenderer/VROFrustum.cpp
             ../../ViroRenderer/VROFrustumPlane.cpp
             ../../ViroRenderer/VROBoundingBox.cpp
             ../../ViroRenderer/VROFrustumBoxIntersectionMetadata.cpp
             ../../ViroRenderer/VROVector3f.cpp
             ../../ViroRenderer/VROVector4f.cpp
             ../../ViroRenderer/VROMatrix4f.cpp
             ../../ViroRenderer/VROMath.cpp
             ../../ViroRenderer/VROProjector.cpp
             ../../ViroRenderer/VRODualQuaternion.cpp

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
             ../../ViroRenderer/VROFBXLoader.cpp
             ../../ViroRenderer/VROModelIOUtil.cpp
             ../../ViroRenderer/VROOBJLoader.cpp
             ../../ViroRenderer/tiny_obj_loader.cc
             ../../ViroRenderer/VROGVRUtil.cpp
             ../../ViroRenderer/VROThreadRestricted.cpp
             ../../ViroRenderer/VROCompress.cpp
             ../../ViroRenderer/Nodes.pb.cc

             # Renderer
             ../../ViroRenderer/VROScene.cpp
             ../../ViroRenderer/VROCamera.cpp
             ../../ViroRenderer/VRONode.cpp
             ../../ViroRenderer/VROPortal.cpp
             ../../ViroRenderer/VROPortalFrame.cpp
             ../../ViroRenderer/VROGeometry.cpp
             ../../ViroRenderer/VROGeometryElement.cpp
             ../../ViroRenderer/VROGeometrySource.cpp
             ../../ViroRenderer/VROMaterial.cpp
             ../../ViroRenderer/VROMaterialVisual.cpp
             ../../ViroRenderer/VROTexture.cpp
             ../../ViroRenderer/VROLight.cpp
             ../../ViroRenderer/VROBillboardConstraint.cpp
             ../../ViroRenderer/VROTransformConstraint.cpp
             ../../ViroRenderer/VRORenderer.cpp
             ../../ViroRenderer/VROFrameSynchronizerInternal.cpp
             ../../ViroRenderer/VROPortalTraversalListener.cpp
             ../../ViroRenderer/VROGeometrySubstrate.cpp
             ../../ViroRenderer/VROTextureSubstrate.cpp
             ../../ViroRenderer/VROMaterialSubstrate.cpp
             ../../ViroRenderer/VRONodeCamera.cpp
             ../../ViroRenderer/VROInputControllerBase.cpp
             ../../ViroRenderer/VROFrameScheduler.cpp
             ../../ViroRenderer/VROChoreographer.cpp
             ../../ViroRenderer/VRORenderPass.cpp
             ../../ViroRenderer/VROPortalTreeRenderPass.cpp
             ../../ViroRenderer/VRORenderTargetOpenGL.cpp

             # Animation
             ../../ViroRenderer/VROTimingFunction.cpp
             ../../ViroRenderer/VROTransaction.cpp
             ../../ViroRenderer/VROAnimatable.cpp
             ../../ViroRenderer/VROAction.cpp
             ../../ViroRenderer/VROAnimationChain.cpp
             ../../ViroRenderer/VROAnimationGroup.cpp
             ../../ViroRenderer/VROPropertyAnimation.cpp
             ../../ViroRenderer/VROSkinner.cpp
             ../../ViroRenderer/VROSkeleton.cpp
             ../../ViroRenderer/VROBone.cpp
             ../../ViroRenderer/VROBoneUBO.cpp
             ../../ViroRenderer/VROSkeletalAnimation.cpp
             ../../ViroRenderer/VROKeyframeAnimation.cpp

             # Shapes
             ../../ViroRenderer/VROBox.cpp
             ../../ViroRenderer/VROSkybox.cpp
             ../../ViroRenderer/VROSurface.cpp
             ../../ViroRenderer/VROTorusKnot.cpp
             ../../ViroRenderer/VROShapeUtils.cpp
             ../../ViroRenderer/VROSphere.cpp
             ../../ViroRenderer/VROPolyline.cpp
             ../../ViroRenderer/VROVideoSurface.cpp
             ../../ViroRenderer/VROPencil.cpp

             # UI
             ../../ViroRenderer/VROReticle.cpp
             ../../ViroRenderer/VROText.cpp
             ../../ViroRenderer/VROKnuthPlassFormatter.cpp
             ../../ViroRenderer/VRODebugHUD.cpp

             # OpenGL
             ../../ViroRenderer/VROGeometrySubstrateOpenGL.cpp
             ../../ViroRenderer/VROMaterialSubstrateOpenGL.cpp
             ../../ViroRenderer/VROTextureSubstrateOpenGL.cpp
             ../../ViroRenderer/VROUniform.cpp
             ../../ViroRenderer/VROShaderProgram.cpp
             ../../ViroRenderer/VROShaderModifier.cpp
             ../../ViroRenderer/VROLightingUBO.cpp
             ../../ViroRenderer/VROGlyphOpenGL.cpp

             # GVR Audio
             ../../ViroRenderer/VROSoundGVR.cpp
             ../../ViroRenderer/VROSoundDataGVR.cpp

             # Physics
             ../../ViroRenderer/VROPhysicsWorld.cpp
             ../../ViroRenderer/VROPhysicsBody.cpp
             ../../ViroRenderer/VROPhysicsShape.cpp

             # Image Tracking
             ../../ViroRenderer/VROImageTracker.cpp
             ../../ViroRenderer/VROImageTrackerOutput.cpp
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

add_library( lib-ovr
             SHARED
             IMPORTED )
set_target_properties( lib-ovr
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libvrapi.so )

add_library( lib-freetype
             SHARED
             IMPORTED )
set_target_properties( lib-freetype
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libfreetype.so )

add_library( lib-protobuf
             SHARED
             IMPORTED )
set_target_properties( lib-protobuf
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libprotobuf-lite.so )

add_library( lib-BulletCollision
             SHARED
             IMPORTED )
set_target_properties( lib-BulletCollision
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libBulletCollision.so )
add_library( lib-BulletDynamics
             SHARED
             IMPORTED )
set_target_properties( lib-BulletDynamics
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libBulletDynamics.so )

add_library( lib-BulletSoftBody
             SHARED
             IMPORTED )
set_target_properties( lib-BulletSoftBody
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libBulletSoftBody.so )

add_library( lib-LinearMath
             SHARED
             IMPORTED )
set_target_properties( lib-LinearMath
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libLinearMath.so )

# -- OpenCV Libraries --

add_library( lib-OpenCV-Java3
             SHARED
             IMPORTED )
set_target_properties (lib-OpenCV-Java3
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so )


add_library( lib-OpenCV-Calib3D
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Calib3D
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_calib3d.a )

add_library( lib-OpenCV-Core
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Core
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_core.a )

add_library( lib-OpenCV-Features2D
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Features2D
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_features2d.a )

add_library( lib-OpenCV-Flann
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Flann
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_flann.a )

add_library( lib-OpenCV-HighGUI
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-HighGUI
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_highgui.a )

add_library( lib-OpenCV-ImgCodecs
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-ImgCodecs
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_imgcodecs.a )

add_library( lib-OpenCV-ImgProc
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-ImgProc
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_imgproc.a )

add_library( lib-OpenCV-ML
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-ML
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_ml.a )

add_library( lib-OpenCV-ObjDetect
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-ObjDetect
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_objdetect.a )

add_library( lib-OpenCV-Photo
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Photo
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_photo.a )

add_library( lib-OpenCV-Shape
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Shape
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_shape.a )

add_library( lib-OpenCV-Stitching
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Stitching
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_stitching.a )

add_library( lib-OpenCV-SuperRes
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-SuperRes
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_superres.a )

add_library( lib-OpenCV-Video
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-Video
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_video.a )

add_library( lib-OpenCV-VideoIO
             STATIC
             IMPORTED )
set_target_properties (lib-OpenCV-VideoIO
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_videoio.a )

add_library( lib-OpenCV-VideoStab
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-VideoStab
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libopencv_videostab.a )

# -- OpenCV 3rd Party Libraries --

add_library( lib-OpenCV-TP-IlmImf
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-IlmImf
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libIlmImf.a )

add_library( lib-OpenCV-TP-Jasper
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-Jasper
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/liblibjasper.a )

add_library( lib-OpenCV-TP-JPEG
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-JPEG
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/liblibjpeg.a )

add_library( lib-OpenCV-TP-PNG
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-PNG
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/liblibpng.a )

add_library( lib-OpenCV-TP-TIFF
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-TIFF
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/liblibtiff.a )

add_library( lib-OpenCV-TP-WebP
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-WebP
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/liblibwebp.a )

add_library( lib-OpenCV-TP-TBB
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-TBB
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libtbb.a )

add_library( lib-OpenCV-TP-Tegra-Hal
               STATIC
               IMPORTED )
set_target_properties (lib-OpenCV-TP-Tegra-Hal
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../../app/src/main/jniLibs/${ANDROID_ABI}/libtegra_hal.a )

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
find_library( lib-z z )

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
                       ${lib-z}
                       lib-gvr
                       lib-gvr-audio
                       lib-ovr
                       lib-freetype
                       lib-protobuf
                       lib-freetype
                       lib-BulletCollision
                       lib-BulletDynamics
                       lib-BulletSoftBody
                       lib-LinearMath

                       # OpenCV Libs
                       lib-OpenCV-Java3
                       lib-OpenCV-Calib3D
                       lib-OpenCV-Core
                       lib-OpenCV-Features2D
                       lib-OpenCV-Flann
                       lib-OpenCV-HighGUI
                       lib-OpenCV-ImgCodecs
                       lib-OpenCV-ImgProc
                       lib-OpenCV-ML
                       lib-OpenCV-ObjDetect
                       lib-OpenCV-Photo
                       lib-OpenCV-Shape
                       lib-OpenCV-Stitching
                       lib-OpenCV-SuperRes
                       lib-OpenCV-Video
                       lib-OpenCV-VideoIO
                       lib-OpenCV-VideoStab

                       # OpenCV 3rd Party Libs
                       lib-OpenCV-TP-IlmImf
                       lib-OpenCV-TP-Jasper
                       lib-OpenCV-TP-JPEG
                       lib-OpenCV-TP-PNG
                       lib-OpenCV-TP-TIFF
                       lib-OpenCV-TP-WebP
                       lib-OpenCV-TP-TBB
                       lib-OpenCV-TP-Tegra-Hal)
