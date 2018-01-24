//
//  VROIrradianceRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROIrradianceRenderPass_h
#define VROIrradianceRenderPass_h

#include "VRORenderPass.h"

class VRODriver;
class VROTexture;
class VROShaderProgram;
class VROImagePostProcess;

const std::string kIrradianceLightingEnvironmentInput = "IR_Input";

/*
 Creates an irradiance map through convolution of an environment map.
 */
class VROIrradianceRenderPass : public VRORenderPass, public std::enable_shared_from_this<VROIrradianceRenderPass> {
public:
    
    VROIrradianceRenderPass();
    virtual ~VROIrradianceRenderPass();
    
    void render(std::shared_ptr<VROScene> scene,
                std::shared_ptr<VROScene> outgoingScene,
                VRORenderPassInputOutput &inputs,
                VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
        
private:
   
    unsigned int _cubeVAO = 0;
    unsigned int _cubeVBO = 0;
    std::shared_ptr<VROShaderProgram> _shader;
    std::shared_ptr<VRORenderTarget> _irradianceRenderTarget;
    
    void init(std::shared_ptr<VRODriver> driver);
    
};

#endif /* VROIrradianceRenderPass_h */
