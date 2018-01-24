//
//  VROEquirectangularToCubeRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROEquirectangularToCubeRenderPass_h
#define VROEquirectangularToCubeRenderPass_h

#include "VRORenderPass.h"

class VRODriver;
class VROTexture;
class VROShaderProgram;
class VROImagePostProcess;

/*
 Keys for the equi-to-cube render pass.
 */
const std::string kEquiToCubeInput = "G_Input";
const std::string kEquiToCubeOutput = "G_PPA";

/*
 Pass that renders an equirectangular HDR image to a cube-map.
 */
class VROEquirectangularToCubeRenderPass : public VRORenderPass, public std::enable_shared_from_this<VROEquirectangularToCubeRenderPass> {
public:
    
    VROEquirectangularToCubeRenderPass(std::shared_ptr<VROTexture> hdrTexture);
    virtual ~VROEquirectangularToCubeRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene,
                                    std::shared_ptr<VROScene> outgoingScene,
                                    VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
    std::shared_ptr<VROTexture> getCubeTexture();
    
private:
   
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;
    std::shared_ptr<VROShaderProgram> _shader;
    std::shared_ptr<VRORenderTarget> _cubeRenderTarget;
    std::shared_ptr<VROTexture> _hdrTexture;
    
    void init(std::shared_ptr<VRODriver> driver);
    bool bindTexture(int unit, const std::shared_ptr<VROTexture> &texture,
                     std::shared_ptr<VRODriver> &driver);
    void renderCube();
    
};

#endif /* VROEquirectangularToCubeRenderPass_h */
