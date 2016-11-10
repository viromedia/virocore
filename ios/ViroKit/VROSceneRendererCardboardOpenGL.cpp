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
    
    _driver = std::make_shared<VRODriverOpenGLiOS>(context);
}

VROSceneRendererCardboardOpenGL::~VROSceneRendererCardboardOpenGL() {
    
}

void VROSceneRendererCardboardOpenGL::initRenderer(GVRHeadTransform *headTransform) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VROSceneRendererCardboardOpenGL::setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController) {
    _renderer->setSceneController(sceneController, *_driver);
}

void VROSceneRendererCardboardOpenGL::setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController, bool animated) {
    _renderer->setSceneController(sceneController, animated, *_driver);
}

void VROSceneRendererCardboardOpenGL::setSceneController(std::shared_ptr<VROSceneControllerInternal> sceneController, float seconds,
                                                         VROTimingFunctionType timingFunctionType) {
    
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, *_driver);
}

void VROSceneRendererCardboardOpenGL::prepareFrame(VROViewport viewport, VROFieldOfView fov,
                                                   GVRHeadTransform *headTransform) {
    VROMatrix4f headRotation = VROConvert::toMatrix4f([headTransform headPoseInStartSpace]).invert();
    _renderer->prepareFrame(_frame, viewport, fov, headRotation, *_driver.get());
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
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
                                                                                            far:kZFar]);

    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    
    VROEyeType eyeType = (eye == kGVRLeftEye ? VROEyeType::Left : VROEyeType::Right);
    _renderer->renderEye(eyeType, eyeMatrix, projectionMatrix, *_driver.get());
}

void VROSceneRendererCardboardOpenGL::endFrame() {
    _renderer->endFrame(*_driver.get());
    ++_frame;
}

void VROSceneRendererCardboardOpenGL::setSuspended(bool suspended) {
    _suspended = suspended;
}
