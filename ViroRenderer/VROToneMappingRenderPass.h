//
//  VROToneMappingRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 8/21/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROToneMappingRenderPass_h
#define VROToneMappingRenderPass_h

#include "VRORenderPass.h"
#include "VROAnimatable.h"

class VRODriver;
class VROImagePostProcess;

const std::string kToneMappingHDRInput = "TM_Input";
const std::string kToneMappingOutput = "TM_Output";

enum class VROToneMappingType {
    Reinhard,
    Exposure
};

/*
 Implements tone mapping for HDR post-processing. Tone mapping
 maps floating point color values into the RGB [0,1] range in a
 manner that preserves image details in both bright and dark
 regions.
 */
class VROToneMappingRenderPass : public VRORenderPass, public VROAnimatable {
public:
    
    VROToneMappingRenderPass(VROToneMappingType type, bool gammaCorrectSoftware,
                             std::shared_ptr<VRODriver> driver);
    virtual ~VROToneMappingRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Set the exposure for the tone-mapping curve. Animatable.
     */
    void setExposure(float exposure);
    
private:

    VROToneMappingType _type;
    float _exposure;
    bool _gammaCorrectionEnabled;
    
    std::shared_ptr<VROImagePostProcess> _postProcess;
    std::shared_ptr<VROImagePostProcess> createPostProcess(std::shared_ptr<VRODriver> driver);
    
};

#endif /* VROToneMappingRenderPass_h */
