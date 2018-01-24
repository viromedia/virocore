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
#include "VRORenderTarget.h"
#include "VROEquirectangularToCubeRenderPass.h"
#include "VROIrradianceRenderPass.h"

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
        
        VRORenderPassInputOutput inputs;
        inputs.textures[kEquirectangularToCubeHDRTextureInput] = _currentLightingEnvironment;
        VROEquirectangularToCubeRenderPass equiToCube;
        equiToCube.render(scene, nullptr, inputs, context, driver);
        
        inputs.textures[kIrradianceLightingEnvironmentInput] = inputs.outputTarget->getTexture(0);
        VROIrradianceRenderPass irradiancePass;
        irradiancePass.render(scene, nullptr, inputs, context, driver);
        
        std::shared_ptr<VROTexture> irradianceMap = inputs.outputTarget->getTexture(0);
        scene->getRootNode()->setBackgroundCube(irradianceMap);
    }
    
}
