cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(GCC_COVERAGE_COMPILE_FLAGS "-DWASM_PLATFORM")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

find_package(OPENGL REQUIRED)

include_directories(../ViroRenderer
                    src/cpp
	            ${OPENGL_INCLUDE_DIR}
                    #libs/freetype/include
                    #libs/protobuf/include
                    libs/bullet/include
		    )

add_library( viro_renderer
             STATIC

             # Web Platform
             src/cpp/VROViewScene.cpp
             src/cpp/VROInputControllerWasm.cpp
             src/cpp/VROImageWasm.cpp

             # Util
             ../ViroRenderer/VROAllocationTracker.cpp
             ../ViroRenderer/VROTime.cpp
             ../ViroRenderer/VROLog.cpp
             ../ViroRenderer/VROByteBuffer.cpp
             ../ViroRenderer/VROImageUtil.cpp
             ../ViroRenderer/VROData.cpp
             ../ViroRenderer/VROGeometryUtil.cpp
             ../ViroRenderer/VROTextureUtil.cpp
             ../ViroRenderer/VROStringUtil.cpp
             ../ViroRenderer/VROPlatformUtil.cpp
             ../ViroRenderer/VROModelIOUtil.cpp
             ../ViroRenderer/VROOBJLoader.cpp
             ../ViroRenderer/VROHDRLoader.cpp
             ../ViroRenderer/tiny_obj_loader.cc
             ../ViroRenderer/VROThreadRestricted.cpp
             ../ViroRenderer/VRORenderUtil.cpp
             ../ViroRenderer/VROTaskQueue.cpp

	     # Renderer
             ../ViroRenderer/VROScene.cpp
             ../ViroRenderer/VROSceneController.cpp
             ../ViroRenderer/VROCamera.cpp
             ../ViroRenderer/VRONode.cpp
             ../ViroRenderer/VROPortal.cpp
             ../ViroRenderer/VROPortalFrame.cpp
             ../ViroRenderer/VROGeometry.cpp
             ../ViroRenderer/VROGeometryElement.cpp
             ../ViroRenderer/VROGeometrySource.cpp
             ../ViroRenderer/VROMaterial.cpp
             ../ViroRenderer/VROMaterialVisual.cpp
             ../ViroRenderer/VROTexture.cpp
             ../ViroRenderer/VROLight.cpp
             ../ViroRenderer/VROBillboardConstraint.cpp
             ../ViroRenderer/VROTransformConstraint.cpp
             ../ViroRenderer/VRORenderer.cpp
             ../ViroRenderer/VROFrameSynchronizerInternal.cpp
             ../ViroRenderer/VROPortalTraversalListener.cpp
             ../ViroRenderer/VROGeometrySubstrate.cpp
             ../ViroRenderer/VROTextureSubstrate.cpp
             ../ViroRenderer/VROMaterialSubstrate.cpp
             ../ViroRenderer/VRONodeCamera.cpp
             ../ViroRenderer/VROInputControllerBase.cpp
             ../ViroRenderer/VROFrameScheduler.cpp
             ../ViroRenderer/VROChoreographer.cpp
             ../ViroRenderer/VROPortalTreeRenderPass.cpp
             ../ViroRenderer/VRORenderTargetOpenGL.cpp
             ../ViroRenderer/VROImageShaderProgram.cpp
             ../ViroRenderer/VROImagePostProcessOpenGL.cpp
             ../ViroRenderer/VROParticleEmitter.cpp
             ../ViroRenderer/VROParticleUBO.cpp
             ../ViroRenderer/VROShadowPreprocess.cpp
             ../ViroRenderer/VROShadowMapRenderPass.cpp
             ../ViroRenderer/VROShaderFactory.cpp
             ../ViroRenderer/VROShaderCapabilities.cpp
             ../ViroRenderer/VROTextureReference.cpp
             ../ViroRenderer/VROMaterialShaderBinding.cpp
             ../ViroRenderer/VROPostProcessEffectFactory.cpp
             ../ViroRenderer/VROToneMappingRenderPass.cpp
             ../ViroRenderer/VROGaussianBlurRenderPass.cpp
             ../ViroRenderer/VROIBLPreprocess.cpp
             ../ViroRenderer/VROEquirectangularToCubeRenderPass.cpp
             ../ViroRenderer/VROIrradianceRenderPass.cpp
             ../ViroRenderer/VROPrefilterRenderPass.cpp
             ../ViroRenderer/VROBRDFRenderPass.cpp
             ../ViroRenderer/VROPointCloudEmitter.cpp

             # UI
             ../ViroRenderer/VROReticle.cpp

             # Physics
             ../ViroRenderer/VROPhysicsWorld.cpp
             ../ViroRenderer/VROPhysicsBody.cpp
             ../ViroRenderer/VROPhysicsShape.cpp

             # AR
             ../ViroRenderer/VROARSceneController.cpp
             ../ViroRenderer/VROARScene.cpp
             ../ViroRenderer/VROARNode.cpp
             ../ViroRenderer/VROARConstraintMatcher.cpp
             ../ViroRenderer/VROARDeclarativeSession.cpp
             ../ViroRenderer/VROARImperativeSession.cpp
             ../ViroRenderer/VROInputControllerAR.cpp
             ../ViroRenderer/VROARShadow.cpp

             # Animation
             ../ViroRenderer/VROTimingFunction.cpp
             ../ViroRenderer/VROTransaction.cpp
             ../ViroRenderer/VROAnimatable.cpp
             ../ViroRenderer/VROAction.cpp
             ../ViroRenderer/VROAnimationChain.cpp
             ../ViroRenderer/VROAnimationGroup.cpp
             ../ViroRenderer/VROPropertyAnimation.cpp
             ../ViroRenderer/VROSkinner.cpp
             ../ViroRenderer/VROSkeleton.cpp
             ../ViroRenderer/VROBone.cpp
             ../ViroRenderer/VROBoneUBO.cpp
             ../ViroRenderer/VROSkeletalAnimation.cpp
             ../ViroRenderer/VROKeyframeAnimation.cpp

             # Shapes
             ../ViroRenderer/VROBox.cpp
             ../ViroRenderer/VROSkybox.cpp
             ../ViroRenderer/VROSurface.cpp
             ../ViroRenderer/VROTorusKnot.cpp
             ../ViroRenderer/VROShapeUtils.cpp
             ../ViroRenderer/VROSphere.cpp
             ../ViroRenderer/VROPolyline.cpp
             ../ViroRenderer/VROVideoSurface.cpp
             ../ViroRenderer/VROPencil.cpp

             # OpenGL
             ../ViroRenderer/VROGeometrySubstrateOpenGL.cpp
             ../ViroRenderer/VROMaterialSubstrateOpenGL.cpp
             ../ViroRenderer/VROTextureSubstrateOpenGL.cpp
             ../ViroRenderer/VROUniform.cpp
             ../ViroRenderer/VROShaderProgram.cpp
             ../ViroRenderer/VROShaderModifier.cpp
             ../ViroRenderer/VROLightingUBO.cpp
	     
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
             ../ViroRenderer/VROVector4f.cpp
)    

add_subdirectory(libs/bullet/src/LinearMath)
add_subdirectory(libs/bullet/src/BulletSoftBody)
add_subdirectory(libs/bullet/src/BulletDynamics)
add_subdirectory(libs/bullet/src/BulletCollision)

add_executable( viro_test
	        hello.cpp)

MESSAGE( STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR} )

set_target_properties( viro_test PROPERTIES LINK_FLAGS "-s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_SOURCE_DIR}/resources@/ --shell-file ${CMAKE_SOURCE_DIR}/src/viro_shell.html --emrun")
target_link_libraries ( viro_test 
                        viro_renderer 
                        bullet_linearmath
                        bullet_softbody
                        bullet_dynamics
                        bullet_collision )