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

enum class VROIBLPhase {
    Idle,
    CubeConvert,
    IrradianceConvolution,
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
    std::shared_ptr<VROTexture> _currentLightingEnvironment;
    std::shared_ptr<VROTexture> _cubeLightingEnvironment;
    std::shared_ptr<VROTexture> _irradianceMap;
    
    void doCubeConversionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                               std::shared_ptr<VRODriver> driver);
    void doIrradianceConvolutionPhase(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                      std::shared_ptr<VRODriver> driver);
    
};

#endif /* VROIBLPreprocess_h */
