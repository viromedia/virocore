//
//  Created by Raj Advani on 5/19/18.
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

#include <memory>
#include <arcore/VROARHitTestResultARCore.h>
#include <arcore/VROARAnchorARCore.h>
#include "VROARImageTarget.h"
#include "VROPlatformUtil.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARHitTestResult_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARHitTestResult_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROARNode), nativeCreateAnchoredNode)(VRO_ARGS
                                                         VRO_REF(VROARHitTestResultARCore) hit_j) {

    std::shared_ptr<VROARHitTestResultARCore> hit = VRO_REF_GET(VROARHitTestResultARCore, hit_j);
    // This should never be called on a hit result with an existing anchor
    passert (hit->getAnchor() == nullptr);

    std::shared_ptr<VROARNode> node = hit->createAnchoredNodeAtHitLocation();
    if (node) {
        return VRO_REF_NEW(VROARNode, node);
    } else {
        return 0;
    }
}

VRO_METHOD(void, nativeDestroyARHitTestResult)(VRO_ARGS
                                               VRO_REF(VROARHitTestResultARCore) hit_j) {
    VRO_REF_DELETE(VROARHitTestResultARCore, hit_j);
}

}