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
#include "VRODriverOpenGL.h"
#include "VROViewport.h"
#include "VROEye.h"

VROSceneRendererCardboardOpenGL::VROSceneRendererCardboardOpenGL(std::shared_ptr<VRORenderer> renderer) :
    _frame(0),
    _renderer(renderer) {
    
    _driver = std::make_shared<VRODriverOpenGL>();
}

VROSceneRendererCardboardOpenGL::~VROSceneRendererCardboardOpenGL() {
    
}

void VROSceneRendererCardboardOpenGL::initRenderer(GCSHeadTransform *headTransform) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void VROSceneRendererCardboardOpenGL::prepareFrame(GCSHeadTransform *headTransform) {
    VROMatrix4f headRotation = matrix_float4x4_from_GL([headTransform headPoseInStartSpace]);
    _renderer->prepareFrame(_frame, headRotation, *_driver.get());
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

void VROSceneRendererCardboardOpenGL::renderEye(GCSEye eye, GCSHeadTransform *headTransform) {
    CGRect rect = [headTransform viewportForEye:eye];
    VROViewport viewport(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    
    VROMatrix4f eyeMatrix = matrix_float4x4_from_GL([headTransform eyeFromHeadMatrix:eye]);
    VROMatrix4f projectionMatrix = matrix_float4x4_from_GL([headTransform projectionMatrixForEye:eye near:0.01 far:100]); //TODO Near far
    
    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    glScissor(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    
    VROEyeType eyeType = (eye == kGCSLeftEye ? VROEyeType::Left : VROEyeType::Right);
    _renderer->renderEye(eyeType, eyeMatrix, projectionMatrix, *_driver.get());
}

void VROSceneRendererCardboardOpenGL::endFrame() {
    _renderer->endFrame(*_driver.get());
}