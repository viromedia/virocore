//
// Created by Raj Advani on 4/25/18.
//

#include "TestAPI.h"
#include "VROLog.h"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Node_##method_name
#elif VRO_PLATFORM_WASM
#define VRO_METHOD(return_type, method_name) \
    return_type method_name
#endif


extern "C" {

VRO_METHOD(VRO_REF, nativeTestMethod) ( VRO_NO_ARGS emscripten::val object ) {
    pinfo("Invoked C method");
    object.call<void>("callJSMethod", 0);
    return 0;
}

EMSCRIPTEN_BINDINGS(TestAPI) {
    emscripten::function("nativeTestMethod", &nativeTestMethod);
}

}