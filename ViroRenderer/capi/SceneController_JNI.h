//
//  SceneController_JNI.h
//  ViroRenderer
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
#include <VROSceneController.h>
#include <VROPlatformUtil.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class SceneControllerDelegate : public VROSceneController::VROSceneControllerDelegate {
public:
    SceneControllerDelegate(VRO_OBJECT obj, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(obj)) {
    }

    ~SceneControllerDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    void onSceneWillAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneWillDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
private:
    void callVoidFunctionWithName(std::string functionName);
    VRO_OBJECT _javaObject;
};
