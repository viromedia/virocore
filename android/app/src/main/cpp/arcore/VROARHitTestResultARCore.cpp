//
//  VROARHitTestResultARCore.h
//  ViroKit
//
//  Created by Raj Advani on 5/19/18
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROARHitTestResultARCore.h"
#include "VROARAnchorARCore.h"
#include "VROARSessionARCore.h"
#include "VROPlatformUtil.h"

VROARHitTestResultARCore::VROARHitTestResultARCore(VROARHitTestResultType type,
                                                   float distance,
                                                   std::shared_ptr<arcore::HitResult> hitResult,
                                                   VROMatrix4f worldTransform, VROMatrix4f localTransform,
                                                   std::shared_ptr<VROARSessionARCore> session) :
    VROARHitTestResult(type, nullptr, distance, worldTransform, localTransform),
    _hitResult(hitResult),
    _session(session) {

}

VROARHitTestResultARCore::~VROARHitTestResultARCore() {

}

std::shared_ptr<VROARAnchor> VROARHitTestResultARCore::createAnchorAtHitLocation(std::shared_ptr<VROARNode> node) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return nullptr;
    }

    // Acquire an ARCore anchor
    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(_hitResult->acquireAnchor());

    // Create a Viro|ARCore anchor
    std::string key = VROStringUtil::toString64(anchor_arc->getId());
    std::shared_ptr<VROARAnchorARCore> anchor = std::make_shared<VROARAnchorARCore>(key, anchor_arc, nullptr, session);
    node->setAnchor(anchor);

    // Set the node on the anchor. Disable thread restriction as we're on the UI thread and this
    // sets the initial rotation, scale, and position of the node.
    node->setThreadRestrictionEnabled(false);
    anchor->setARNode(node);
    node->setThreadRestrictionEnabled(true);

    // Sync the anchor's transforms and add it to session for updates
    anchor->sync();

    // Adding anchors to the session requires the rendering thread
    VROPlatformDispatchAsyncRenderer([session, anchor] {
        session->addAnchor(anchor);
    });

    _anchor = anchor;
    return anchor;
}
