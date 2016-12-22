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
#include "VROEventDelegate.h"
#include <memory>
#include <float.h>

static const float kReticleSizeMultiple = 3;

class VROReticleSizeListener : public VROEventDelegate {
public:
    
    VROReticleSizeListener(std::shared_ptr<VROReticle> reticle,
                           std::shared_ptr<VRORenderContext> context) :
            _previousHoverDepth(0),
            _context(context),
            _reticle(reticle) {
    }

    /*VROEventDelegate*/
    void onGazeHitDistance(float distance){
        float depth = -distance;

        if (fabs(_previousHoverDepth - depth) < kEpsilon) {
            return;
        }

        std::shared_ptr<VRORenderContext> context = _context.lock();
        std::shared_ptr<VROReticle> reticle = _reticle.lock();
        if (!reticle || !context) {
            return;
        }

        _previousHoverDepth = depth;
        
        float worldPerScreen = context->getCamera().getWorldPerScreen(depth);
        float radius = worldPerScreen * kReticleSizeMultiple;
        reticle->setDepth(depth);
        reticle->setRadius(radius);
    }
    
private:
    std::weak_ptr<VRORenderContext> _context;
    std::weak_ptr<VROReticle> _reticle;
    float _previousHoverDepth;

};

#endif /* VROReticleSizeListener_h */
