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
#include "VROScreenUIView.h"
#include "VROReticle.h"
#include "VROMath.h"
#include "VROView.h"

static const float kReticleSizeMultiple = 3;

class VROReticleSizeListener : public VROHoverDistanceListener {
    
public:
    
    VROReticleSizeListener(id <VROView> view) :
        _previousHoverDepth(0) {
        _view = view;
    }
    
    void onHoverDistanceChanged(float distance) {
        float depth = -distance;
        if (distance == FLT_MAX) {
            depth = -5;
        }
        if (fabs(_previousHoverDepth - depth) < kEpsilon) {
            return;
        }
        _previousHoverDepth = depth;
        
        float worldPerScreen = [_view worldPerScreenAtDepth:depth];
        float radius = worldPerScreen * kReticleSizeMultiple;
        
        VROReticle *reticle = _view.reticle;
        reticle->setDepth(depth);
        reticle->setRadius(radius);
    }
    
private:
    
    __weak id <VROView> _view;
    float _previousHoverDepth;

};

#endif /* VROReticleSizeListener_h */
