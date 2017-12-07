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

enum class VROToneMappingMethod {
    Disabled,
    Reinhard,
    Exposure,
    Hable,
    HableLuminanceOnly,
};

/*
 Implements tone mapping for HDR post-processing. Tone mapping
 maps floating point color values into the RGB [0,1] range in a
 manner that preserves image details in both bright and dark
 regions.
 */
class VROToneMappingRenderPass : public VRORenderPass, public VROAnimatable {
public:
    
    VROToneMappingRenderPass(VROToneMappingMethod method, bool gammaCorrectSoftware,
                             std::shared_ptr<VRODriver> driver);
    virtual ~VROToneMappingRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene,
                                    std::shared_ptr<VROScene> outgoingScene,
                                    VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Set the tone mapping method to use. This will regenerate the post-process
     effect.
     */
    void setMethod(VROToneMappingMethod method);
    
    /*
     Set the exposure for the tone-mapping curve. This value is exponential; e.g.
     1.0 doubles brightness, 2.0 quadruples, etc. Animatable.
     
     (Note: if in the future we auto-detect exposure from average scene luminance, we
            will replace this value with minExposure and maxExposure to clamp the
            detected exposure levels).
     */
    void setExposure(float exposure);
    
    /*
     Set the white point for the tone-mapping curve. The white point is the lowest luminance that
     will appear as white. Modifying this property will change contrast. Animatable.
     */
    void setWhitePoint(float whitePoint);
    
private:

    VROToneMappingMethod _method;
    float _exposure;
    float _whitePoint;
    bool _gammaCorrectionEnabled;
    
    std::shared_ptr<VROImagePostProcess> _postProcess;
    std::shared_ptr<VROImagePostProcess> createPostProcess(std::shared_ptr<VRODriver> driver);
    
};

#endif /* VROToneMappingRenderPass_h */
