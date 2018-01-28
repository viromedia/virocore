//
//  VROBRDFRenderPass.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROBRDFRenderPass_h
#define VROBRDFRenderPass_h

#include "VRORenderPass.h"

class VRODriver;
class VROTexture;
class VROShaderProgram;
class VROImagePostProcess;

/*
 Pre-computes and stores an irradiance BRDF map into a 2D lookup texture.
 */
class VROBRDFRenderPass : public VRORenderPass, public std::enable_shared_from_this<VROBRDFRenderPass> {
public:

    VROBRDFRenderPass();
    virtual ~VROBRDFRenderPass();

    void render(std::shared_ptr<VROScene> scene,
                std::shared_ptr<VROScene> outgoingScene,
                VRORenderPassInputOutput &inputs,
                VRORenderContext *context, std::shared_ptr<VRODriver> &driver);

private:
    unsigned int _quadVAO = 0;
    unsigned int _quadVBO = 0;
    std::shared_ptr<VROShaderProgram> _shader;
    std::shared_ptr<VRORenderTarget> _BRDFRenderTarget;
    void init(std::shared_ptr<VRODriver> driver);
};

#endif /* VROBRDFRenderPass_h */
