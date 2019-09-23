//
//  VROARDraggableNodeTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROARDraggableNodeTest.h"
#include "VROTestUtil.h"
#include "VROModelIOUtil.h"

VROARDraggableNodeTest::VROARDraggableNodeTest() :
    VRORendererTest(VRORendererTestType::ARDraggableNode) {
        
}

VROARDraggableNodeTest::~VROARDraggableNodeTest() {
    
}

void VROARDraggableNodeTest::build(std::shared_ptr<VRORenderer> renderer,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {

    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    arScene->initDeclarativeSession();
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    
    std::string url = VROTestUtil::getURLForResource("coffee_mug", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));

    std::shared_ptr<VRONode> objNode = std::make_shared<VRONode>();
    VROOBJLoader::loadOBJFromResource(url, VROResourceType::URL, objNode, driver,
                                      [this](std::shared_ptr<VRONode> node, bool success) {
                                          if (!success) {
                                              return;
                                          }
                                          node->setScale({0.007, 0.007, 0.007});

                                          std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
                                          material->getDiffuse().setTexture(VROTestUtil::loadDiffuseTexture("coffee_mug"));
                                          material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("coffee_mug_specular"));
                                      });
    
    std::shared_ptr<VROBox> box = VROBox::createBox(.15, .15, .15);
    std::shared_ptr<VRONode> draggableNode = std::make_shared<VRONode>();
    
    _eventDelegate = std::make_shared<VROARDraggableNodeEventDelegate>();
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnDrag, true);
    
    draggableNode->setEventDelegate(_eventDelegate);
    draggableNode->setDragType(VRODragType::FixedToWorld);
    
    draggableNode->setPosition(VROVector3f(0,0,-1));
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition(VROVector3f(0, .10, 0));
    draggableNode->addChildNode(boxNode);
    sceneNode->addChildNode(draggableNode);
    arScene->addNode(sceneNode);
    
    // Add a shadow under the box.
    std::shared_ptr<VROTexture> texture = VROTestUtil::loadDiffuseTexture("dark_circle_shadow");
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(.3, .3);
    surface->getMaterials().front()->getDiffuse().setTexture(texture);
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setRotationEulerX(-1.570795); // rotate it so it's facing "up" (-PI/2)
    
    draggableNode->addChildNode(surfaceNode);
}
