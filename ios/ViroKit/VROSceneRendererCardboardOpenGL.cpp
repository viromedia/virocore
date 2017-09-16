//
//  VROSceneRendererCardboardOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSceneRendererCardboardOpenGL.h"
#include "VRORenderer.h"
#include "VROMatrix4f.h"
#include "VRODriverOpenGLiOS.h"
#include "VROViewport.h"
#include "VROEye.h"
#include "VROConvert.h"

VROSceneRendererCardboardOpenGL::VROSceneRendererCardboardOpenGL(EAGLContext *context,
                                                                 std::shared_ptr<VRORenderer> renderer) :
    _frame(0),
    _renderer(renderer),
    _suspended(true) {
    
    _gvrAudio = std::make_shared<gvr::AudioApi>();
    _gvrAudio->Init(GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    _driver = std::make_shared<VRODriverOpenGLiOS>(nil, context, _gvrAudio);
    _baseRotation = VROMatrix4f();
    _baseRotation.toIdentity();
      
      
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord
                                     withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker
                                           error:nil];
   
}

VROSceneRendererCardboardOpenGL::~VROSceneRendererCardboardOpenGL() {
    
}

void VROSceneRendererCardboardOpenGL::initRenderer(GVRHeadTransform *headTransform) {
    glEnable(GL_DEPTH_TEST);
}

void VROSceneRendererCardboardOpenGL::setSceneController(std::shared_ptr<VROSceneController> sceneController) {
    _renderer->setSceneController(sceneController, _driver);
}

void VROSceneRendererCardboardOpenGL::setSceneController(std::shared_ptr<VROSceneController> sceneController, float seconds,
                                                         VROTimingFunctionType timingFunctionType) {
    
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);
}

void VROSceneRendererCardboardOpenGL::prepareFrame(VROViewport viewport, VROFieldOfView fov,
                                                   GVRHeadTransform *headTransform) {

    VROMatrix4f headRotation = VROConvert::toMatrix4f([headTransform headPoseInStartSpace]).invert();

    // TODO: VIRO-1235, the below code works, but GVR should expose a recenterTracking function in their SDK which does the same thing.
    if (_recenterTracking) {
        // get the current head rotation's forward vector
        VROVector3f forward = headRotation.multiply(kBaseForward);
        // zero out the y value/project line onto XZ plane
        forward.y = 0;
        // get the angle in the XZ plane between the base forward and the camera forward
        float theta = kBaseForward.angleWithVector(forward);
        // create a new base rotation with the correct Y rotation to zero out the user's Y rotation
        _baseRotation.toIdentity();
        _baseRotation.rotateY(forward.x < 0 ? -theta : theta);
        // reset _recenterTracking as we only want to do this once until the next time recenterTracking() is called
        _recenterTracking = false;
    }
    headRotation = _baseRotation.multiply(headRotation);
    
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    _renderer->prepareFrame(_frame, viewport, fov, headRotation, projection, _driver);

    glEnable(GL_SCISSOR_TEST); // Ensures we only clear scissored area when using glClear
    glEnable(GL_DEPTH_TEST);
    _driver->setCullMode(VROCullMode::Back);
    // GLKMatrix is a float[16] array, whereas gvr::Mat4f is a float[4][4] array, this is just converting one to the other.
    GLKMatrix4 glkmatrix = [headTransform headPoseInStartSpace];
    gvr::Mat4f matrix = {glkmatrix.m[0], glkmatrix.m[4], glkmatrix.m[8], glkmatrix.m[12]};
    _gvrAudio->SetHeadPose(matrix);
    _gvrAudio->Update();
}

void VROSceneRendererCardboardOpenGL::renderEye(GVREye eye, GVRHeadTransform *headTransform) {
    if (_suspended) {
        return;
    }
    
    CGRect rect = [headTransform viewportForEye:eye];
    VROViewport viewport(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    
    VROMatrix4f eyeMatrix = VROConvert::toMatrix4f([headTransform eyeFromHeadMatrix:eye]);
    VROMatrix4f projectionMatrix = VROConvert::toMatrix4f([headTransform projectionMatrixForEye:eye
                                                                                           near:kZNear
                                                                                            far:_renderer->getFarClippingPlane()]);
    
    VROEyeType eyeType = (eye == kGVRLeftEye ? VROEyeType::Left : VROEyeType::Right);
    _renderer->renderEye(eyeType, eyeMatrix, projectionMatrix, viewport, _driver);
}

void VROSceneRendererCardboardOpenGL::endFrame() {
    _renderer->endFrame(_driver);
    ++_frame;
}

void VROSceneRendererCardboardOpenGL::setSuspended(bool suspended) {
    _suspended = suspended;
}

void VROSceneRendererCardboardOpenGL::recenterTracking() {
    _recenterTracking = true;
}
