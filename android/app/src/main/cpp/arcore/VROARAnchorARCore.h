//
//  VROARAnchorARCore.h
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
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

#ifndef VROARAnchorARCore_h
#define VROARAnchorARCore_h

#include "VROARAnchor.h"
#include "ARCore_API.h"

class VROARSessionARCore;

/*
 Superclass for all ARCore anchors. Unlike ARKit, where trackables and anchors are a merged
 concept, in ARCore they are separate. In ARCore, we check for updated trackables each frame
 in the VROARSessionARCore. For each trackable found, we create an ARAnchor corresponding to it:
 these are known as "managed" anchors. Managed anchors own the actual internal (ARCore) anchor,
 and maintain a link to the corresponding trackable (which is *also* a subclass of VRARAnchor).
 */
class VROARAnchorARCore : public VROARAnchor {
public:
    
    VROARAnchorARCore(std::string key,
                      std::shared_ptr<arcore::Anchor> anchor,
                      std::shared_ptr<VROARAnchor> trackable,
                      std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARAnchorARCore();

    /*
     Managed anchors are anchors that are automatically created by Viro and assigned to each
     detected trackable.
     */
    bool isManaged() const {
        return _trackable != nullptr;
    }

    /*
     ARCore anchors use a different anchor for the actual trackable.
     */
    std::shared_ptr<VROARAnchor> getAnchorForTrackable() {
        if (_trackable) {
            return _trackable;
        } else {
            return shared_from_this();
        }
    }

    /*
     Sync this anchor with the latest data in its underlying ARCore anchor.
     */
    void sync();

    /*
     Detach this anchor from the session, so that it no longer receives updates. This will
     also remove the corresponding ARNode from the scene.

     Invoke from rendering thread.
     */
    void detach();

    /*
     Get the cloud identifier for this anchor. This is only valid for anchors that have been
     hosted to the cloud or resolved from the cloud.
     */
    std::string getCloudAnchorId() const;
    void loadCloudAnchorId();

    std::shared_ptr<VROARAnchor> getTrackable();
    std::shared_ptr<arcore::Anchor> getAnchorInternal();

private:

    std::shared_ptr<arcore::Anchor> _anchor;
    std::shared_ptr<VROARAnchor> _trackable;
    std::weak_ptr<VROARSessionARCore> _session;
    std::string _cloudAnchorId;

};

#endif /* VROARAnchorARCore_h */
