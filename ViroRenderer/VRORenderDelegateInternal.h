//
//  VRORenderDelegateInternal.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRORenderDelegateInternal_h
#define VRORenderDelegateInternal_h

enum class VROEyeType;
class VRORenderContext;
class VROVector3f;
class VRODriver;

class VRORenderDelegateInternal {
    
public:
    
    VRORenderDelegateInternal() {}
    virtual ~VRORenderDelegateInternal() {}
    
    virtual void setupRendererWithDriver(std::shared_ptr<VRODriver> driver) = 0;
    virtual void userDidRequestExitVR() = 0;
    
};

#endif /* VRORenderDelegateInternal_h */
