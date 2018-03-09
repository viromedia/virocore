//
//  VROARImperativeSession.h
//  ViroKit
//
//  Created by Raj Advani on 11/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARImperativeSession_h
#define VROARImperativeSession_h

#include <vector>
#include "VROARSession.h"

class VROARScene;
class VROARImperativeSessionDelegate;

class VROARImperativeSession : public VROARSessionDelegate, public std::enable_shared_from_this<VROARImperativeSession> {
public:
    
    VROARImperativeSession(std::shared_ptr<VROARScene> scene);
    virtual ~VROARImperativeSession();

    // TODO: the following 3 functions are also in VROARDeclarativeSession
    void setARSession(std::shared_ptr<VROARSession> session);
    void addARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> target);

    // VROARSessionDelegate methods
    virtual void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor);
    virtual void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor);
    virtual void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor);
    virtual void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor);

    void setDelegate(std::shared_ptr<VROARImperativeSessionDelegate> delegate) {
        _delegate = delegate;
    }
    
private:

    std::weak_ptr<VROARSession> _arSession;
    std::weak_ptr<VROARScene> _scene;
    std::weak_ptr<VROARImperativeSessionDelegate> _delegate;
    std::vector<std::shared_ptr<VROARImageTarget>> _imageTargets;

};

class VROARImperativeSessionDelegate {
public:
    
    /*
     Invoked whenever an anchor is detected by the AR session, with the node that the imperative
     session has created to correspond with the anchor. The node stays in sync with the anchor
     and is already a part of the scene.
     */
    virtual void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) = 0;
    
    /*
     Invoked just before and after the node's properties are updated to match the current state of
     the anchor.
     */
    virtual void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) = 0;
    virtual void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) = 0;
    
    /*
     Invoked when an anchor is removed from the AR session, along with its corresponding node (now detached
     from the scene).
     */
    virtual void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) = 0;
    
};

#endif /* VROARImperativeSession_h */
