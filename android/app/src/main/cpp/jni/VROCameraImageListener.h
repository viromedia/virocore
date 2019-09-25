//
//  Created by Raj Advani on 7/7/18.
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

#ifndef ANDROID_VROCAMERAIMAGELISTENER_H
#define ANDROID_VROCAMERAIMAGELISTENER_H

#include <memory>
#include "VRODefines.h"
#include VRO_C_INCLUDE

#include "VROPlatformUtil.h"
#include "VROFrameListener.h"
#include "VROARScene.h"

class VROSceneRendererARCore;

class VROCameraImageFrameListener : public VROFrameListener {
public:
    VROCameraImageFrameListener(VRO_OBJECT listener_j, std::shared_ptr<VROSceneRendererARCore> renderer, VRO_ENV env) :
            _listener_j(VRO_NEW_WEAK_GLOBAL_REF(listener_j)),
            _renderer(renderer),
            _bufferIndex(0) {

        for (int i = 0; i < 3; i++) {
            _buffers[i] = NULL;
        }
    }

    virtual ~VROCameraImageFrameListener() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_listener_j);

        for (int i = 0; i < 3; i++) {
            if (_buffers[i] != NULL) {
                VRO_DELETE_GLOBAL_REF(_buffers[i]);
            }
        }
    }

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:
    VRO_OBJECT _listener_j;
    std::weak_ptr<VROSceneRendererARCore> _renderer;
    int _bufferIndex;
    std::shared_ptr<VROData> _data[3];
    jobject _buffers[3];
};


#endif //ANDROID_VROCAMERAIMAGELISTENER_H
