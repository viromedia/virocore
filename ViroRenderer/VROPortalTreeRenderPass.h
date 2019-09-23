//
//  VROPortalTreeRenderPass.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROPortalTreeRenderPass_h
#define VROPortalTreeRenderPass_h

#include "VRORenderPass.h"
#include "VROTree.h"

class VRODriver;
class VROPortal;
class VROMaterial;

class VROPortalTreeRenderPass : public VRORenderPass {
public:
    
    VROPortalTreeRenderPass();
    virtual ~VROPortalTreeRenderPass();
    
    void render(std::shared_ptr<VROScene> scene,
                std::shared_ptr<VROScene> outgoingScene,
                VRORenderPassInputOutput &inputs,
                VRORenderContext *context, std::shared_ptr<VRODriver> &driver);
    
private:
    
    /*
     Material used to render silhouettes of objects to the scene.
     */
    std::shared_ptr<VROMaterial> _silhouetteMaterial;
    
    /*
     Helper function for rendering. Performs depth-first rendering of portals, rendering
     the portal silhouettes to the stencil buffer on the way down, and the portal geometry
     and content on the way up.
     */
    void render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes,
                std::shared_ptr<VROPortal> outgoingTopPortal, bool renderBackgrounds,
                std::shared_ptr<VRORenderTarget> &target,
                const VRORenderContext &context,
                std::shared_ptr<VRODriver> &driver);
};

#endif /* VROPortalTreeRenderPass_h */
