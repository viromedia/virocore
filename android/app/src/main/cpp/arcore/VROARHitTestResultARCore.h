//
//  VROARHitTestResultARCore.h
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

#ifndef ANDROID_VROARHITTESTRESULTARCORE_H
#define ANDROID_VROARHITTESTRESULTARCORE_H

#include "VROARHitTestResult.h"
#include "ARCore_API.h"

class VROARSessionARCore;

class VROARHitTestResultARCore : public VROARHitTestResult, public std::enable_shared_from_this<VROARHitTestResultARCore> {
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
     to the returned ARNode.

     This is expected to be called from the Application thread.
     */
    std::shared_ptr<VROARNode> createAnchoredNodeAtHitLocation();

private:

    std::shared_ptr<arcore::HitResult> _hitResult;
    std::weak_ptr<VROARSessionARCore> _session;

};


#endif //ANDROID_VROARHITTESTRESULTARCORE_H
