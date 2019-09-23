//
//  VROARImperativeSession.h
//  ViroKit
//
//  Created by Raj Advani on 11/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROARImperativeSession_h
#define VROARImperativeSession_h

#include <vector>
#include "VROARSession.h"
#include "VROARImageDatabase.h"

class VROARScene;
class VROARImperativeSessionDelegate;

class VROARImperativeSession : public VROARSessionDelegate, public std::enable_shared_from_this<VROARImperativeSession> {
public:
    
    VROARImperativeSession(std::shared_ptr<VROARScene> scene);
    virtual ~VROARImperativeSession();

    // TODO: the following functions are also in VROARDeclarativeSession
    void setARSession(std::shared_ptr<VROARSession> session);
    void loadARImageDatabase(std::shared_ptr<VROARImageDatabase> arImageDatabase);
    void unloadARImageDatabase();
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
    std::shared_ptr<VROARImageDatabase> _arImageDatabase;

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
