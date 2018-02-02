//
//  VROARPlaneTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

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
    arScene->initDeclarativeSession();
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    std::shared_ptr<VROARDeclarativePlane> arPlane = std::make_shared<VROARDeclarativePlane>(0, 0);
    
    std::string url = VROTestUtil::getURLForResource("coffee_mug", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));

    std::shared_ptr<VRONode> objNode = std::make_shared<VRONode>();
    VROOBJLoader::loadOBJFromResource(url, VROResourceType::URL, objNode, true,
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
}
