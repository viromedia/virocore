//
//  VROPrefilterRenderPass.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROPrefilterRenderPass_h
#define VROPrefilterRenderPass_h

#include "VRORenderPass.h"

class VRODriver;
class VROTexture;
class VROShaderProgram;
class VROImagePostProcess;

const std::string kPrefilterLightingEnvironmentInput = "Prefilter_Input";

/*
 Creates a prefiltered irradiance cubemap through convolution of an environment map.
 */
class VROPrefilterRenderPass : public VRORenderPass, public std::enable_shared_from_this<VROPrefilterRenderPass> {
public:

    VROPrefilterRenderPass();
    virtual ~VROPrefilterRenderPass();

    void render(std::shared_ptr<VROScene> scene,
                std::shared_ptr<VROScene> outgoingScene,
                VRORenderPassInputOutput &inputs,
                VRORenderContext *context, std::shared_ptr<VRODriver> &driver);

private:
    unsigned int _cubeVAO = 0;
    unsigned int _cubeVBO = 0;
    std::shared_ptr<VROShaderProgram> _shader;
    std::shared_ptr<VRORenderTarget> _prefilterRenderTarget;
    void init(std::shared_ptr<VRODriver> driver);
};

#endif /* VROPrefilterRenderPass_h */
