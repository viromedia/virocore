//
//  VROARImperativeSession.h
//  ViroKit
//
//  Created by Raj Advani on 11/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARImperativeSession_h
#define VROARImperativeSession_h

#include "VROARSession.h"

class VROARImperativeSessionDelegate;

class VROARImperativeSession : public VROARSessionDelegate, public std::enable_shared_from_this<VROARImperativeSession> {
public:
    
    VROARImperativeSession();
    virtual ~VROARImperativeSession();
    
    // VROARSessionDelegate methods
    virtual void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor);
    virtual void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor);
    virtual void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor);
    virtual void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor);
    
private:
    
    std::weak_ptr<VROARImperativeSessionDelegate> _delegate;
    
};

class VROARImperativeSessionDelegate {
public:
    
    /*
     Invoked whenever an anchor is detected by the AR session, with the node that the imperative
     session has created to correspond with the anchor. The node stays in sync with the anchor
     and is already a part of the scene.
     */
    virtual void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    
    /*
     Invoked just before and after the node's properties are updated to match the current state of
     the anchor.
     */
    virtual void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    virtual void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    
    /*
     Invoked when an anchor is removed from the AR session, along with its corresponding node (now detached
     from the scene).
     */
    virtual void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    
};

#endif /* VROARImperativeSession_h */
