//
//  Created by Raj Advani on 4/25/18.
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

#include "TestAPI.h"
#include "VROLog.h"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Node_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type TestAPI_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(void), nativeTestMethod)(VRO_NO_ARGS) {
    pinfo("Invoked C method");

    VROPlatformCallHostFunction(obj, "callJSMethod", "", 5, 7);
    return 0;
}

EMSCRIPTEN_BINDINGS(TestAPI) {
    emscripten::function("nativeTestMethod", &TestAPI_nativeTestMethod, emscripten::allow_raw_pointers());
}

}