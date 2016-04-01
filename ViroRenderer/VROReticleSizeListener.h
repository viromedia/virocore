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

static const float kReticleSizeMultiple = 666;

class VROReticleSizeListener : public VROHoverDistanceListener {
    
public:
    
    VROReticleSizeListener(VROView *view) {
        _view = view;
    }
    
    void onHoverDistanceChanged(float distance) {
        float depth = -distance;
        if (distance == FLT_MAX) {
            depth = -8;
        }
        
        NSLog(@"setting reticle distance %f", depth);
        
        float worldPerScreen = [_view worldPerScreenAtDepth:depth];
        
        float size = worldPerScreen * kReticleSizeMultiple;
        float thickness = size / 2.0;
        
        [_view.HUD setDepth:depth];
        [_view.HUD.reticle setReticleSize:size];
        [_view.HUD.reticle setReticleThickness:thickness];
    }
    
private:
    
    __weak VROView *_view;

};

#endif /* VROReticleSizeListener_h */
