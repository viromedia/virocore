//
//  VROARPlaneTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARPlaneTest.h"
#include "VROTestUtil.h"

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
    std::shared_ptr<VROARPlane> arPlane = std::make_shared<VROARPlane>(0, 0);
    
    std::string url = VROTestUtil::getURLForResource("coffee_mug", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));
    
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(url, base, true,
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
    
    // Taking screenshot/video logic:
    // TODO: Restore this after VIRO-1482, which should make screen recording cross-platform
    //       so that we don't need a handle on VROViewAR.
    /*
    VROViewAR *arView = (VROViewAR *)self.view;
    int rand = arc4random_uniform(1000);
    
    BOOL takeVideo = NO;
    if (takeVideo) {
        NSString *filename = [NSString stringWithFormat:@"testvideo%d", rand];
        
        NSLog(@"[VROSample] started recording");
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [arView startVideoRecording:filename saveToCameraRoll:YES errorBlock:nil];
        });
        
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            NSLog(@"[VROSample] stopped recording");
            [arView stopVideoRecordingWithHandler:^(BOOL success, NSURL *url, NSInteger errorCode) {
             if (url) {
             [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
             }
             }];
        });
    } else {
        NSString *filename = [NSString stringWithFormat:@"testimage%d", rand];
        
        NSLog(@"[VROSample] taking screenshot in 5 seconds");
        
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            [arView takeScreenshot:filename saveToCameraRoll:YES withCompletionHandler:^(BOOL success, NSURL *url, NSInteger errorCode) {
             if (url) {
             [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
             }
             }];
        });
    }
     */
}
