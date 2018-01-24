//
//  VROIBLPreprocess.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROIBLPreprocess.h"
#include "VROPortal.h"
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VROEquirectangularToCubeRenderPass.h"

VROIBLPreprocess::VROIBLPreprocess() {
    
}

VROIBLPreprocess::~VROIBLPreprocess() {
    
}

void VROIBLPreprocess::execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                               std::shared_ptr<VRODriver> driver) {
 
    // TODO This is a test method to see if converting equirectangular to cube
    //      maps work. Will be replaced by convolution in future IBL commits.
    
    // In this test method we take the environment lighting set in the active
    // portal and make it the background of the portal
    
    std::shared_ptr<VROPortal> portal = scene->getActivePortal();
    
    if (portal->getLightingEnvironment() != nullptr && portal->getLightingEnvironment() != _currentLightingEnvironment) {
        
        _currentLightingEnvironment = portal->getLightingEnvironment();
        VROEquirectangularToCubeRenderPass equiToCube(_currentLightingEnvironment);
        
        VRORenderPassInputOutput inputs;
        equiToCube.render(scene, nullptr, inputs, context, driver);
        
        std::shared_ptr<VROTexture> cube = equiToCube.getCubeTexture();
        scene->getRootNode()->setBackgroundCube(cube);
    }
    
}
