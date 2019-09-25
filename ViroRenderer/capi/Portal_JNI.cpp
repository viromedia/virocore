//
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

#include <memory>
#include <VROPortalFrame.h>
#include "Portal_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Portal_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Portal_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROPortalFrame), nativeCreatePortal)(VRO_NO_ARGS) {
    std::shared_ptr<VROPortalFrame> portal = std::make_shared<VROPortalFrame>();
    return VRO_REF_NEW(VROPortalFrame, portal);
}

VRO_METHOD(void, nativeDestroyPortal)(VRO_ARGS
                                      VRO_REF(VROPortalFrame) native_ref) {
    VRO_REF_DELETE(VROPortalFrame, native_ref);
}

}