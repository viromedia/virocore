//
//  VROFrameSynchronizerInternal.h
//  ViroRenderer
//
//  Created by Raj Advani on 7/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROFrameSynchronizerInternal_h
#define VROFrameSynchronizerInternal_h

#include "VROFrameSynchronizer.h"
#include <vector>

class VRORenderContext;

class VROFrameSynchronizerInternal : public VROFrameSynchronizer {
    
public:
    
    virtual ~VROFrameSynchronizerInternal() {}
    
    void addFrameListener(std::shared_ptr<VROFrameListener> listener);
    void removeFrameListener(std::shared_ptr<VROFrameListener> listener);
    
    void notifyFrameStart(const VRORenderContext &context);
    void notifyFrameEnd(const VRORenderContext &context);
    
private:
    
    /*
     Listeners that receive an update each frame.
     */
    std::vector<std::weak_ptr<VROFrameListener>> _frameListeners;
    
};

#endif /* VROFrameSynchronizerInternal_h */
