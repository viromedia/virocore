//
//  VROGaussianBlurRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 8/24/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROGaussianBlurRenderPass_h
#define VROGaussianBlurRenderPass_h

#include "VRORenderPass.h"

class VRODriver;
class VROImagePostProcess;

/*
 Keys for the gaussian render pass.
 */
const std::string kGaussianInput = "G_Input";
const std::string kGaussianPingPongA = "G_PPA";
const std::string kGaussianPingPongB = "G_PPB";

/*
 Implements Gaussian blur for Bloom. Ping-pongs the blur between
 two render targets.
 */
class VROGaussianBlurRenderPass : public VRORenderPass, public std::enable_shared_from_this<VROGaussianBlurRenderPass> {
public:
    
    VROGaussianBlurRenderPass();
    virtual ~VROGaussianBlurRenderPass();
    
    VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver);

    /*
     The more iterations, the more blur. Must be an even number.
     */
    void setNumBlurIterations(int numIterations);
    
private:
    
    std::shared_ptr<VROImagePostProcess> _gaussianBlur;
    int _numBlurIterations;
    bool _horizontal;
    void initPostProcess(std::shared_ptr<VRODriver> driver);
    
};



#endif /* VROGaussianBlurRenderPass_h */
