//
// Created by Raj Advani on 7/7/18.
//

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
