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

std::shared_ptr<VROARNode> VROARHitTestResultARCore::createAnchoredNodeAtHitLocation() {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return nullptr;
    }

    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(_hitResult->acquireAnchor());
    std::shared_ptr<VROARNode> node = session->createAnchoredNode(anchor_arc);

    _anchor = node->getAnchor();
    return node;
}
