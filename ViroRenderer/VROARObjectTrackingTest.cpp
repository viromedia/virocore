//
//  VROARObjectTrackingTest.cpp
//  ViroSample
//
//  Created by Andy Chu on 8/9/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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

#include "VROARObjectTrackingTest.h"
#include "VROTestUtil.h"
#include "VROModelIOUtil.h"
#include "VROARObjectTargetiOS.h"
#include "VROARDeclarativeObjectNode.h"

VROARObjectTrackingTest::VROARObjectTrackingTest() :
VRORendererTest(VRORendererTestType::ARPlane) {
    
}

VROARObjectTrackingTest::~VROARObjectTrackingTest() {
    
}

void VROARObjectTrackingTest::build(std::shared_ptr<VRORenderer> renderer,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    
    std::shared_ptr<VROARSceneController> sceneController = std::make_shared<VROARSceneController>();
    _sceneController = sceneController;
    
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    arScene->initDeclarativeSession();
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    
    // there's also the alcohol_bottle and diet_coke arobjects.
    NSURL *objUrl = [[NSBundle mainBundle] URLForResource:@"google_home" withExtension:@"arobject"];

    std::shared_ptr<VROARObjectTargetiOS> objectTarget = std::make_shared<VROARObjectTargetiOS>(objUrl);
    std::shared_ptr<VROARDeclarativeObjectNode> arObjectNode = std::make_shared<VROARDeclarativeObjectNode>();
    arObjectNode->setObjectTarget(objectTarget);
    arObjectNode->setARNodeDelegate(shared_from_this());
    
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
    
    sceneNode->addChildNode(arObjectNode);
    arObjectNode->addChildNode(objNode);
    arScene->getDeclarativeSession()->addARObjectTarget(objectTarget);
    arScene->getDeclarativeSession()->addARNode(arObjectNode);
    
    arScene->addNode(sceneNode);
    
    // After a few seconds, remove the node and then a few seconds later, reattach it!
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        // note, that removing a node doesn't cause onARAnchorRemoved to be called because the user removed
        // the node, but the underlying anchor (real world feature) still exists!
        arScene->getDeclarativeSession()->removeARNode(arObjectNode);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            arScene->getDeclarativeSession()->addARNode(arObjectNode);
        });
    });
}

void VROARObjectTrackingTest::onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor) {
    VROVector3f pos = anchor->getTransform().extractTranslation();
    pinfo("ARImageObjectTest onAnchorAttached! pos: %f %f %f", pos.x, pos.y, pos.z);
}

void VROARObjectTrackingTest::onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) {
    VROVector3f pos = anchor->getTransform().extractTranslation();
    pinfo("ARImageObjectTest onAnchorUpdated! pos: %f %f %f", pos.x, pos.y, pos.z);
}

void VROARObjectTrackingTest::onARAnchorRemoved() {
    pinfo("ARImageObjectTest onAnchorRemoved!");
}
