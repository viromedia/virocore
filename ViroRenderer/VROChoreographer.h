//
//  VROChoreographer.h
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROChoreographer_h
#define VROChoreographer_h

#include <memory>
#include <vector>

class VROScene;
class VRODriver;
class VRORenderPass;
class VRORenderContext;

class VROChoreographer {
public:
    
    VROChoreographer();
    virtual ~VROChoreographer();
    
    virtual void render(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                        std::shared_ptr<VRODriver> &driver);
    
    void setBaseRenderPass(std::shared_ptr<VRORenderPass> pass) {
        _baseRenderPass = pass;
    }
    void setPostProcessRenderPasses(std::vector<std::shared_ptr<VRORenderPass>> passes) {
        _postProcessRenderPasses = passes;
    }
    
private:
    
    std::shared_ptr<VRORenderPass> _baseRenderPass;
    std::vector<std::shared_ptr<VRORenderPass>> _postProcessRenderPasses;
    
};

#endif /* VROChoreographer_h */
