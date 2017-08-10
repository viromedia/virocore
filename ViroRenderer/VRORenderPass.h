//
//  VRORenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRORenderPass_h
#define VRORenderPass_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include <map>

class VROScene;
class VRODriver;
class VRODisplay;
class VROShaderProgram;
class VRORenderContext;
class VRORenderTarget;

/*
 Key to use for render passes that output a single render target.
 This key may also be used in the input map to indicate what input buffer
 to write to and return as an output.
 */
const std::string kRenderTargetSingleOutput = "RT_Output";

/*
 The output of a render pass is a map of render targets. These are typically backed
 by depth, stencil, and or color textures. Each target is keyed by a unique name.
 Inputs to render passes are similarly identified.
 */
typedef std::map<std::string, std::shared_ptr<VRORenderTarget>> VRORenderPassInputOutput;

/*
 Render passes represent a rendering of the scene to produce one or more render targets.
 Render passes take the full 3D scene as input, along with any previously created render
 targets. They can either render the 3D scene to a target, or implement post-processing by
 taking the input targets and creating new output targets.
 
 Render passes are chained together via the VROChoreographer.
 
 Render passes can also be configured to write directly to the display, in which case they
 may return no targets.
 */
class VRORenderPass {
public:
    
    VRORenderPass();
    virtual ~VRORenderPass();
    
    /*
     Perform the render pass. This is implemented by the subclasses.
     
     It is common and expected for render passes to re-use the same output render targets
     each frame.
     */
    virtual VRORenderPassInputOutput render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                            VRORenderContext *context, std::shared_ptr<VRODriver> &driver) = 0;
    
};

#endif /* VRORenderPass_h */
