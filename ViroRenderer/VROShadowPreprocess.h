//
//  VROShadowPreprocess.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROShadowPreprocess_h
#define VROShadowPreprocess_h

#include "VROPreprocess.h"
#include <vector>
#include <map>
#include <functional>

class VROLight;
class VRORenderTarget;
class VROShadowMapRenderPass;

class VROShadowPreprocess : public VROPreprocess {
public:
    
    VROShadowPreprocess(std::shared_ptr<VRODriver> driver);
    virtual ~VROShadowPreprocess() {}
    
    /*
     Render shadows to the shadow maps for each shadow casting light.
     */
    virtual void execute(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                         std::shared_ptr<VRODriver> driver);
    
private:
    
    /*
     The max supported shadow map size for the current device.
     */
    int _maxSupportedShadowMapSize;
    
    /*
     The render target for the shadow passes. This target uses a depth texture array
     to capture shadow maps for all lights.
     */
    std::shared_ptr<VRORenderTarget> _shadowTarget;
    
    /*
     The shadow passes for creating the depth maps for each light.
     */
    std::map<std::shared_ptr<VROLight>, std::shared_ptr<VROShadowMapRenderPass>> _shadowPasses;
    
};

#endif /* VROShadowPreprocess_h */
