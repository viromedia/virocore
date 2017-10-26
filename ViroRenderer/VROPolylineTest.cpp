//
//  VROPolylineTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPolylineTest.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROPolyline.h"
#include "VROToneMappingRenderPass.h"

VROPolylineTest::VROPolylineTest() :
    VRORendererTest(VRORendererTestType::Polyline) {
    
}

VROPolylineTest::~VROPolylineTest() {
    
}

void VROPolylineTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer, std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setBackgroundSphere(VROTestUtil::loadHDRTexture("wooden"));
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    
    _pointOfView = cameraNode;
    
    std::shared_ptr<VRONode> polylineNode = std::make_shared<VRONode>();
    polylineNode->setPosition({ 0, 0, -2 });
    
    _polyline = std::make_shared<VROPolyline>();
    _polyline->setThickness(0.25);
    polylineNode->setGeometry(_polyline);
    
    scene->getRootNode()->addChildNode(polylineNode);
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setPosition({ 0, 0, -2 });
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(10, 10);
    surface->getMaterialForElement(0)->getDiffuse().setColor({0.0, 1.0, 0.0, 1.0});
    surfaceNode->setGeometry(surface);
    scene->getRootNode()->addChildNode(surfaceNode);
    
    _eventDelegate = std::make_shared<VROPolylineEventDelegate>(_polyline);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnMove, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnDrag, true);
    surfaceNode->setEventDelegate(_eventDelegate);
}

void VROPolylineEventDelegate::onClick(int source, std::shared_ptr<VRONode> node,
                                       ClickState clickState, std::vector<float> position) {
    std::shared_ptr<VROPolyline> polyline = _polyline.lock();
    if (clickState == ClickState::Clicked && polyline && position.size() > 2) {
        polyline->appendPoint({ position[0], position[1], 0.1 });
    }
}

void VROPolylineEventDelegate::onMove(int source, std::shared_ptr<VRONode> node,
                                      VROVector3f rotation, VROVector3f position, VROVector3f forwardVec) {
    
}

void VROPolylineEventDelegate::onDrag(int source, std::shared_ptr<VRONode> node,
                                      VROVector3f position) {
    
}

