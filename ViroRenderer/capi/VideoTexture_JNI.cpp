//
//  VideoTexture_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <memory>
#include <VROVideoSurface.h>
#include <VROTextureUtil.h>
#include "VRONode.h"
#include "VROMaterial.h"
#include "PersistentRef.h"
#include "VROFrameSynchronizer.h"
#include "VROPlatformUtil.h"
#include "ViroContext_JNI.h"
#include "VideoTexture_JNI.h"
#include "VideoDelegate_JNI.h"
#include "VRODriverOpenGL.h"

#if VRO_PLATFORM_ANDROID
#include "VROVideoTextureAVP.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_VideoTexture_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateVideoTexture)(VRO_ARGS
                                              VRO_REF context_j,
                                              VRO_STRING stereoMode) {
    VROStereoMode mode = VROTextureUtil::getStereoModeForString(VROPlatformGetString(stereoMode, env));
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    std::shared_ptr<VROVideoTexture> videoTexture;
#if VRO_PLATFORM_ANDROID
    std::shared_ptr<VROVideoTextureAVP> videoAVP = std::make_shared<VROVideoTextureAVP>(mode);
    videoAVP->init();

    VROPlatformDispatchAsyncRenderer([videoAVP, context_w] {
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!context) {
            return;
        }
        videoAVP->bindSurface(std::dynamic_pointer_cast<VRODriverOpenGL>(context->getDriver()));
    });
    videoTexture = videoAVP;
#else
    //TODO wasm
#endif

    return VideoTexture::jptr(videoTexture);
}

VRO_METHOD(VRO_REF, nativeCreateVideoDelegate)(VRO_NO_ARGS) {

    std::shared_ptr<VideoDelegate> delegate = std::make_shared<VideoDelegate>(obj);
    return VideoDelegate::jptr(delegate);
}

VRO_METHOD(void, nativeAttachDelegate)(VRO_ARGS
                                       VRO_REF textureRef, VRO_REF delegateRef) {

    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    std::weak_ptr<VideoDelegate> videoDelegate_w = VideoDelegate::native(delegateRef);

    VROPlatformDispatchAsyncRenderer([videoTexture_w, videoDelegate_w] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        std::shared_ptr<VideoDelegate> videoDelegate = videoDelegate_w.lock();
        if (!videoDelegate) {
            return;
        }

        videoTexture->setDelegate(videoDelegate);
        videoDelegate->onReady();
    });
}

VRO_METHOD(void, nativeDeleteVideoTexture)(VRO_ARGS
                                           VRO_REF textureRef) {
    delete reinterpret_cast<PersistentRef<VROVideoTexture> *>(textureRef);
}

VRO_METHOD(void, nativeDeleteVideoDelegate)(VRO_ARGS
                                            VRO_REF delegateRef) {
    delete reinterpret_cast<PersistentRef<VideoDelegate> *>(delegateRef);
}

VRO_METHOD(void, nativePause)(VRO_ARGS
                              VRO_REF textureRef) {
    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->pause();
    });
}

VRO_METHOD(void, nativePlay)(VRO_ARGS
                             VRO_REF textureRef) {

    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->play();
    });
}

VRO_METHOD(void, nativeSetMuted)(VRO_ARGS
                                 VRO_REF textureRef,
                                 VRO_BOOL muted) {

    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, muted] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->setMuted(muted);
    });
}

VRO_METHOD(void, nativeSetVolume)(VRO_ARGS
                                  VRO_REF textureRef,
                                  VRO_FLOAT volume) {

    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, volume] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->setVolume(volume);
    });
}

VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                VRO_REF textureRef,
                                VRO_BOOL loop) {

    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, loop] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->setLoop(loop);
    });
}

VRO_METHOD(void, nativeSeekToTime)(VRO_ARGS
                                   VRO_REF textureRef,
                                   VRO_FLOAT seconds) {

    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    VROPlatformDispatchAsyncRenderer([videoTexture_w, seconds] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        videoTexture->seekToTime(seconds);
    });
}

VRO_METHOD(void, nativeLoadSource)(VRO_ARGS
                                   VRO_REF textureRef,
                                   VRO_STRING source,
                                   VRO_REF context_j) {
    // Grab required objects from the RenderContext required for initialization
    std::string strVideoSource = VROPlatformGetString(source, env);
    std::weak_ptr<VROVideoTexture> videoTexture_w = VideoTexture::native(textureRef);
    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    VROPlatformDispatchAsyncRenderer([videoTexture_w, context_w, strVideoSource] {
        std::shared_ptr<VROVideoTexture> videoTexture = videoTexture_w.lock();
        if (!videoTexture) {
            return;
        }
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!context) {
            return;
        }

        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer = context->getFrameSynchronizer();
        std::shared_ptr<VRODriver> driver = context->getDriver();
        videoTexture->loadVideo(strVideoSource, frameSynchronizer, driver);
        videoTexture->prewarm();
    });
}

}  // extern "C"