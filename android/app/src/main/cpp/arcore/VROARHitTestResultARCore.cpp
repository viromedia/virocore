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

    std::shared_ptr<VROARNode> node = std::make_shared<VROARNode>();

    // We have to immediately set the position and rotation of the node so that this data
    // is available on the Application (UI) thread. This node is added to the root node so
    // we can compute its transforms with identity parent matrices.
    float matrix[16];
    _hitResult->getTransform(matrix);
    VROMatrix4f transform(matrix);

    VROVector3f position = transform.extractTranslation();
    VROVector3f scale = transform.extractScale();
    VROQuaternion rotation = transform.extractRotation(scale);

    node->setPositionAtomic(position);
    node->setRotationAtomic(rotation);
    node->computeTransformsAtomic({}, {});

    // Acquire the anchor from the hit result. If tracking is limited then this can
    // fail, in which case we return null.
    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(_hitResult->acquireAnchor());
    if (anchor_arc) {
        std::weak_ptr<VROARSessionARCore> session_w = session;
        VROPlatformDispatchAsyncRenderer([session_w, anchor_arc, node] {
            std::shared_ptr<VROARSessionARCore> session_s = session_w.lock();
            if (!session_s) {
                return;
            }
            session_s->addManualAnchor(anchor_arc, node);
        });
        return node;
    } else {
        pinfo("Failed to create anchor from hit result: no anchored node will be created");
        return nullptr;
    }
}
