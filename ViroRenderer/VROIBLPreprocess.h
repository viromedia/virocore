//
//  VROIBLPreprocess.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROIBLPreprocess_h
#define VROIBLPreprocess_h

#include "VROPreprocess.h"

class VROTexture;
class VROEquirectangularToCubeRenderPass;
class VROIrradianceRenderPass;
class VROPrefilterRenderPass;
class VROBRDFRenderPass;

enum class VROIBLPhase {
    Idle,
    CubeConvert,
    IrradianceConvolution,
    PrefilterConvolution,
    BRDFConvolution
};

class VROIBLPreprocess : public VROPreprocess {
public:
    VROIBLPreprocess();
    virtual ~VROIBLPreprocess();
    
    virtual void execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                         std::shared_ptr<VRODriver> driver);
    
private:
    
    VROIBLPhase _phase;
    std::shared_ptr<VROEquirectangularToCubeRenderPass> _equirectangularToCubePass;
    std::shared_ptr<VROIrradianceRenderPass> _irradiancePass;
    std::shared_ptr<VROPrefilterRenderPass> _prefilterPass;
    std::shared_ptr<VROBRDFRenderPass> _brdfPass;

    std::shared_ptr<VROTexture> _currentLightingEnvironment;
    std::shared_ptr<VROTexture> _cubeLightingEnvironment;
    std::shared_ptr<VROTexture> _irradianceMap;
    std::shared_ptr<VROTexture> _prefilterMap;
    std::shared_ptr<VROTexture> _brdfMap;

    void doCubeConversionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                               std::shared_ptr<VRODriver> driver);
    void doIrradianceConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                      std::shared_ptr<VRODriver> driver);
    void doPrefilterConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                     std::shared_ptr<VRODriver> driver);
    void doBRDFComputationPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                     std::shared_ptr<VRODriver> driver);
};

#endif /* VROIBLPreprocess_h */
