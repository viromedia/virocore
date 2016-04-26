//
//  VRODriverCardboard.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverCardboard_h
#define VRODriverCardboard_h

#include "VRODriver.h"
#include "GCSCardboardView.h"

class VRODriverCardboard : public VRODriver {
    
public:
    
    VRODriverCardboard();
    virtual ~VRODriverCardboard();
    
    virtual UIView *getRenderingView();
    virtual void onOrientationChange(UIInterfaceOrientation orientation);
    
    virtual VROViewport getViewport(VROEyeType eye);
    virtual VROFieldOfView getFOV(VROEyeType eye);

private:
    
};

#endif /* VRODriverCardboard_h */
