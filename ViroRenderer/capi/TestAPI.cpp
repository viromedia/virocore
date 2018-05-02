//
// Created by Raj Advani on 4/25/18.
//

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

VRO_METHOD(VRO_REF, nativeTestMethod)(VRO_NO_ARGS) {
    pinfo("Invoked C method");

    VROPlatformCallHostFunction(obj, "callJSMethod", "", 5, 7);
    return 0;
}

EMSCRIPTEN_BINDINGS(TestAPI) {
    emscripten::function("nativeTestMethod", &TestAPI_nativeTestMethod, emscripten::allow_raw_pointers());
}

}