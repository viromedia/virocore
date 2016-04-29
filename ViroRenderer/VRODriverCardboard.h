//
//  VRODriverCardboard.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverCardboard_h
#define VRODriverCardboard_h

#import "VRODriver.h"
#import "GCSCardboardView.h"

class VRORenderer;
class VRODriverContextMetal;
@class VROCardboardRenderLoop;

class VRODriverCardboard : public VRODriver {
    
public:
    
    VRODriverCardboard(std::shared_ptr<VRORenderer> renderer);
    virtual ~VRODriverCardboard();
    
    virtual void onOrientationChange(UIInterfaceOrientation orientation);
    
    virtual VROViewport getViewport(VROEyeType eye);
    virtual VROFieldOfView getFOV(VROEyeType eye);
    
private:
    
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VRODriverContextMetal> _context;
    
    
};

#endif /* VRODriverCardboard_h */
