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
    
    VROToneMappingRenderPass(VROToneMappingType type, std::shared_ptr<VRODriver> driver);
    virtual ~VROToneMappingRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Set the exposure for the tone-mapping curve. Animatable.
     */
    void setExposure(float exposure);
    
    /*
     Set to true to manually gamma correct the image during tone-mapping.
     */
    void setGammaCorrectionEnabled(bool enabled);
    
private:

    VROToneMappingType _type;
    float _exposure;
    bool _gammaCorrectionEnabled;
    
    std::shared_ptr<VROImagePostProcess> _postProcessHDR;
    std::shared_ptr<VROImagePostProcess> _postProcessHDRAndGamma;
    
    std::shared_ptr<VROImagePostProcess> createPostProcess(std::shared_ptr<VRODriver> driver,
                                                           bool gammaCorrect);
    
};

#endif /* VROToneMappingRenderPass_h */
