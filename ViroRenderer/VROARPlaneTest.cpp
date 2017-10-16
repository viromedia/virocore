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

VROARPlaneTest::VROARPlaneTest() :
    VRORendererTest(VRORendererTestType::ARPlane) {
        
}

VROARPlaneTest::~VROARPlaneTest() {
    
}

void VROARPlaneTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           std::shared_ptr<VRODriver> driver) {
    
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    std::shared_ptr<VROARPlaneNode> arPlane = std::make_shared<VROARPlaneNode>(0, 0);
    
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
    arScene->addARPlane(arPlane);
    arScene->addNode(sceneNode);
}
