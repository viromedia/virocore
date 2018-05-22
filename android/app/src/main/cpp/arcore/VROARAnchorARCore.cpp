//
//  VROARAnchorARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/24/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARAnchorARCore.h"
#include "VROMatrix4f.h"
#include "VROARSessionARCore.h"
#include "VROPlatformUtil.h"

VROARAnchorARCore::VROARAnchorARCore(std::string key,
                                     std::shared_ptr<arcore::Anchor> anchor,
                                     std::shared_ptr<VROARAnchor> trackable,
                                     std::shared_ptr<VROARSessionARCore> session) :
    _anchor(anchor),
    _trackable(trackable),
    _session(session) {
    setId(key);
}

VROARAnchorARCore::~VROARAnchorARCore() {
    // When this shared_ptr is destroyed, we can safely detach the anchor. This is because
    // at this point we know that the anchor has been removed from the VROARSessionARCore
    // if its a managed anchor (an anchor bound to a trackable) or it has been
    // manually detached from its ARNode if it's a manual anchor (anchored to a hit
    // result).
    std::shared_ptr<arcore::Anchor> anchor = _anchor;
    VROPlatformDispatchAsyncRenderer([anchor] {
        if (anchor) {
            anchor->detach();
        }
    });
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
