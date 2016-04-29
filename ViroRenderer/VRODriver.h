//
//  VRODriver.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriver_h
#define VRODriver_h

#include <memory>
#import <UIKit/UIKit.h> //TODO Delete

class VRORenderer;
class VROFieldOfView;
class VROViewport;
enum class VROEyeType;

class VRODriver {
    
public:
    
    VRODriver();
    virtual ~VRODriver();
    
    virtual void onOrientationChange(UIInterfaceOrientation orientation) = 0;
    virtual VROViewport getViewport(VROEyeType eye) = 0;
    virtual VROFieldOfView getFOV(VROEyeType eye) = 0;
    
private:
    
};


#endif /* VRODriver_h */
