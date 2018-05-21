//
//  VROARHitTestResultARCore.h
//  ViroKit
//
//  Created by Raj Advani on 5/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROARHITTESTRESULTARCORE_H
#define ANDROID_VROARHITTESTRESULTARCORE_H

#include "VROARHitTestResult.h"
#include "ARCore_API.h"

class VROARSessionARCore;

class VROARHitTestResultARCore : public VROARHitTestResult {
public:

    VROARHitTestResultARCore(VROARHitTestResultType type,
                             float distance,
                             std::shared_ptr<arcore::HitResult> hitResult,
                             VROMatrix4f worldTransform, VROMatrix4f localTransform,
                             std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARHitTestResultARCore();

    std::shared_ptr<arcore::HitResult> getHitResultInternal() const {
        return _hitResult;
    }

    /*
     Create an ARAnchor at the position of this hit result, and add the anchor to the
     VROARSessionARCore for continued tracking and updates. The anchor will be assigned
     to the provided ARNode.

     This is expected to be called from the Application thread.
     */
    std::shared_ptr<VROARAnchor> createAnchorAtHitLocation(std::shared_ptr<VROARNode> node);

private:

    std::shared_ptr<arcore::HitResult> _hitResult;
    std::weak_ptr<VROARSessionARCore> _session;

};


#endif //ANDROID_VROARHITTESTRESULTARCORE_H
