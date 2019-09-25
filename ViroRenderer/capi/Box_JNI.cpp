//
//  Box_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
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
#include <VROPlatformUtil.h>
#include "VROBox.h"
#include "VROMaterial.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Box_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Box_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROBox), nativeCreateBox)(VRO_ARGS
                                             VRO_FLOAT width,
                                             VRO_FLOAT height,
                                             VRO_FLOAT length) {
    std::shared_ptr<VROBox> box = VROBox::createBox(width, height, length);
    return VRO_REF_NEW(VROBox, box);
}

VRO_METHOD(void, nativeSetWidth)(VRO_ARGS
                                 VRO_REF(VROBox) native_box_ref,
                                 VRO_FLOAT width) {
    std::weak_ptr<VROBox> box_w = VRO_REF_GET(VROBox, native_box_ref);
    VROPlatformDispatchAsyncRenderer([box_w, width] {
        std::shared_ptr<VROBox> box = box_w.lock();
        if (box) {
            box->setWidth(width);
        }
    });
}

VRO_METHOD(void, nativeSetHeight)(VRO_ARGS
                                  VRO_REF(VROBox) native_box_ref,
                                  VRO_FLOAT height) {
    std::weak_ptr<VROBox> box_w = VRO_REF_GET(VROBox, native_box_ref);
    VROPlatformDispatchAsyncRenderer([box_w, height] {
        std::shared_ptr<VROBox> box = box_w.lock();
        if (box) {
            box->setHeight(height);
        }
    });
}

VRO_METHOD(void, nativeSetLength)(VRO_ARGS
                                  VRO_REF(VROBox) native_box_ref,
                                  VRO_FLOAT length) {
    std::weak_ptr<VROBox> box_w = VRO_REF_GET(VROBox, native_box_ref);
    VROPlatformDispatchAsyncRenderer([box_w, length] {
        std::shared_ptr<VROBox> box = box_w.lock();
        if (box) {
            box->setLength(length);
        }
    });
}

}  // extern "C"
