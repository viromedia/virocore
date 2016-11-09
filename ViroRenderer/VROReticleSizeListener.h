//
//  VROReticleSizeListener.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/1/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROReticleSizeListener_h
#define VROReticleSizeListener_h

#include "VROHoverDistanceListener.h"
#include "VROReticle.h"
#include "VROMath.h"
#include <memory>

static const float kReticleSizeMultiple = 3;

class VROReticleSizeListener : public VROHoverDistanceListener {
    
public:
    
    VROReticleSizeListener(std::shared_ptr<VROReticle> reticle) :
        _previousHoverDepth(0) {
        _reticle = reticle;
    }
    
    void onHoverDistanceChanged(float distance, const VRORenderContext &context) {
        float depth = -distance;
        if (distance == FLT_MAX) {
            depth = -5;
        }
        if (fabs(_previousHoverDepth - depth) < kEpsilon) {
            return;
        }
        
        std::shared_ptr<VROReticle> reticle = _reticle.lock();
        if (!reticle) {
            return;
        }
        
        _previousHoverDepth = depth;
        
        float worldPerScreen = context.getCamera().getWorldPerScreen(depth);
        float radius = worldPerScreen * kReticleSizeMultiple;
        
        reticle->setDepth(depth);
        reticle->setRadius(radius);
    }
    
private:
    
    std::weak_ptr<VROReticle> _reticle;
    float _previousHoverDepth;

};

#endif /* VROReticleSizeListener_h */
