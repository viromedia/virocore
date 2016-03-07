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

class VROHoverController : public VROFrameListener {
    
public:
    
    VROHoverController(float rotationThresholdRadians, std::shared_ptr<VROScene> scene,
                       bool hitTestBoundsOnly,
                       std::function<bool(std::shared_ptr<VRONode> node)> isHoverable,
                       std::function<void(std::shared_ptr<VRONode> node)> onHoverOn,
                       std::function<void(std::shared_ptr<VRONode> node)> onHoverOff);
    virtual ~VROHoverController();
    
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);
    
private:
    
    std::weak_ptr<VROScene> _scene;
    std::weak_ptr<VRONode> _hoveredNode;
    bool _hitTestBoundsOnly;
    
    std::function<bool(std::shared_ptr<VRONode> node)> _isHoverable;
    std::function<void(std::shared_ptr<VRONode> node)> _hoverOn, _hoverOff;
    
    const float _rotationThresholdRadians;
    VROVector3f _lastCameraForward;
    
    void findHoveredNode(VROVector3f ray, std::shared_ptr<VROScene> &scene);
    
};

#endif /* VROHoverController_h */
