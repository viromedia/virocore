//
//  VROARPlaneTest.cpp
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

#include "VROARPlaneTest.h"
#include "VROTestUtil.h"
#include "VROModelIOUtil.h"
#include "VROARDeclarativePlane.h"

VROARPlaneTest::VROARPlaneTest() :
    VRORendererTest(VRORendererTestType::ARPlane) {
        
}

VROARPlaneTest::~VROARPlaneTest() {
    
}

void VROARPlaneTest::build(std::shared_ptr<VRORenderer> renderer,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           std::shared_ptr<VRODriver> driver) {
    
    std::shared_ptr<VROARSceneController> sceneController = std::make_shared<VROARSceneController>();
    _sceneController = sceneController;

    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    std::set<VROAnchorDetection> types;
    types.insert(VROAnchorDetection::PlanesHorizontal);
    arScene->setAnchorDetectionTypes(types);
    arScene->initDeclarativeSession();
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    std::shared_ptr<VROARDeclarativePlane> arPlane = std::make_shared<VROARDeclarativePlane>(0, 0, VROARPlaneAlignment::Horizontal);
    arPlane->setARNodeDelegate(shared_from_this());

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

    sceneNode->addChildNode(arPlane);
    arPlane->addChildNode(objNode);

    arScene->getDeclarativeSession()->addARNode(arPlane);
    arScene->addNode(sceneNode);

#if VRO_PLATFORM_IOS
    bool testRemoval = false;
        if (testRemoval) {
        // After a few seconds, remove the node and then a few seconds later, reattach it!
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            // note, that removing a node doesn't cause onARAnchorRemoved to be called because the user removed
            // the node, but the underlying anchor (real world feature) still exists!
            arScene->getDeclarativeSession()->removeARNode(arPlane);
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                arScene->getDeclarativeSession()->addARNode(arPlane);
            });
        });
    }
#endif
}

void VROARPlaneTest::onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor) {
    VROVector3f pos = anchor->getTransform().extractTranslation();
    pinfo("ARPlaneTest onAnchorAttached! pos: %f %f %f", pos.x, pos.y, pos.z);
}

void VROARPlaneTest::onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) {
    VROVector3f pos = anchor->getTransform().extractTranslation();
    pinfo("ARPlaneTest onAnchorUpdated! pos: %f %f %f", pos.x, pos.y, pos.z);
}

void VROARPlaneTest::onARAnchorRemoved() {
    pinfo("ARPlaneTest onAnchorRemoved!");
}
