//
//  VROARAnchorARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/24/17.
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

#include "VROARAnchorARCore.h"
#include "VROMatrix4f.h"
#include "VROARSessionARCore.h"
#include "VROPlatformUtil.h"
#include "VROAllocationTracker.h"

VROARAnchorARCore::VROARAnchorARCore(std::string key,
                                     std::shared_ptr<arcore::Anchor> anchor,
                                     std::shared_ptr<VROARAnchor> trackable,
                                     std::shared_ptr<VROARSessionARCore> session) :
    _anchor(anchor),
    _trackable(trackable),
    _session(session) {
    setId(key);

    ALLOCATION_TRACKER_ADD(Anchors, 1);
}

VROARAnchorARCore::~VROARAnchorARCore() {
    // When this shared_ptr is destroyed, we can safely detach the anchor. This is because
    // at this point we know that the anchor has been removed from the VROARSessionARCore
    // if its a managed anchor (an anchor bound to a trackable) or it has been
    // manually detached from its ARNode if it's a manual anchor (anchored to a hit
    // result).
    std::shared_ptr<arcore::Anchor> anchor = _anchor;
    std::weak_ptr<VROARSessionARCore> session = _session;
    VROPlatformDispatchAsyncRenderer([anchor, session] {
        if (anchor && session.lock()) {
            anchor->detach();
        }
    });

    ALLOCATION_TRACKER_SUB(Anchors, 1);
}

void VROARAnchorARCore::sync() {
    float mtx[16];
    _anchor->getTransform(mtx);
    setTransform({mtx});
}

std::string VROARAnchorARCore::getCloudAnchorId() const {
    return _cloudAnchorId;
}

void VROARAnchorARCore::loadCloudAnchorId() {
    char *cloudAnchorId;
    _anchor->acquireCloudAnchorId(&cloudAnchorId);
    _cloudAnchorId = std::string(cloudAnchorId);
    // TODO Release the generated cloudAnchorId by exposing ArString_release()
}

void VROARAnchorARCore::detach() {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }
    // Only manual anchors (those not tied to trackables) can be manually detached
    if (isManaged()) {
        return;
    }
    session->removeAnchor(shared_from_this());
}

std::shared_ptr<arcore::Anchor> VROARAnchorARCore::getAnchorInternal() {
    return _anchor;
}

std::shared_ptr<VROARAnchor> VROARAnchorARCore::getTrackable() {
    return _trackable;
}
