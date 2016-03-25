//
//  VROHoverController.h
//  ViroRenderer
//
//  Created by Raj Advani on 2/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROHoverController_h
#define VROHoverController_h

#include "VROFrameListener.h"
#include "VROQuaternion.h"
#include <functional>

class VROScene;
class VRONode;
class VROHoverDelegate;

class VROHoverController : public VROFrameListener {
    
public:
    
    VROHoverController(float rotationThresholdRadians,
                       std::shared_ptr<VROScene> scene);
    virtual ~VROHoverController();
    
    void setDelegate(std::shared_ptr<VROHoverDelegate> delegate);
    
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);
    
private:
    
    std::weak_ptr<VROScene> _scene;
    std::weak_ptr<VRONode> _hoveredNode;
    std::weak_ptr<VROHoverDelegate> _delegate;
    
    const float _rotationThresholdRadians;
    VROVector3f _lastCameraForward;
    
    void findHoveredNode(VROVector3f ray, std::shared_ptr<VROScene> &scene,
                         const VRORenderContext &context);
    
};

#endif /* VROHoverController_h */
