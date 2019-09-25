//
//  Created by Raj Advani on 10/24/17.
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

#include "Light_JNI.h"
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Light_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Light_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeDestroyLight)(VRO_ARGS
                                     VRO_REF(VROLight) native_light_ref) {
    VRO_REF_DELETE(VROLight, native_light_ref);
}

VRO_METHOD(void, nativeSetColor)(VRO_ARGS
                                 VRO_REF(VROLight) native_light_ref,
                                 VRO_LONG color) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);

    VROPlatformDispatchAsyncRenderer([light_w, color] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        // Get the color
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8) & 0xFF) / 255.0;
        float b = (color & 0xFF) / 255.0;

        VROVector3f vecColor(r, g, b);
        light->setColor(vecColor);
    });
}

VRO_METHOD(void, nativeSetIntensity)(VRO_ARGS
                                     VRO_REF(VROLight) native_light_ref,
                                     VRO_FLOAT intensity) {
    std::shared_ptr<VROLight> light = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, intensity] {
        light->setIntensity(intensity);
    });
}

VRO_METHOD(void, nativeSetTemperature)(VRO_ARGS
                                       VRO_REF(VROLight) native_light_ref,
                                       VRO_FLOAT temperature) {
    std::shared_ptr<VROLight> light = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light, temperature] {
        light->setTemperature(temperature);
    });
}

VRO_METHOD(void, nativeSetInfluenceBitMask)(VRO_ARGS
                                            VRO_REF(VROLight) native_light_ref,
                                            VRO_INT bitMask) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, native_light_ref);
    VROPlatformDispatchAsyncRenderer([light_w, bitMask] {
        std::shared_ptr<VROLight> light = light_w.lock();
        if (!light) {
            return;
        }
        light->setInfluenceBitMask(bitMask);
    });
}

}




