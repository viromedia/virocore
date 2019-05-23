//
//  VROBodyMesher.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/23/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROBodyMesher_h
#define VROBodyMesher_h

#include "VROVisionModel.h"

enum class VROCameraPosition;

class VROBodyMesherDelegate {
public:
    virtual void onBodyMeshUpdated() = 0;
};

class VROBodyMesher : public VROVisionModel {
    
public:
    VROBodyMesher() {};
    virtual ~VROBodyMesher() {}
    
    virtual bool initBodyTracking(VROCameraPosition position, std::shared_ptr<VRODriver> driver) = 0;
    virtual void startBodyTracking() = 0;
    virtual void stopBodyTracking() = 0;
    
    void setDelegate(std::shared_ptr<VROBodyMesherDelegate> delegate) {
        _bodyMeshDelegate_w = delegate;
    }
    
    /*
     Sets the window period at which we sample points for dampening. If period == 0,
     no dampening will be applied.
     */
    virtual void setDampeningPeriodMs(double period) {}
    virtual double getDampeningPeriodMs() const { return 0; }
    
protected:
    std::weak_ptr<VROBodyMesherDelegate> _bodyMeshDelegate_w;
    
};

#endif /* VROBodyMesher_h */
