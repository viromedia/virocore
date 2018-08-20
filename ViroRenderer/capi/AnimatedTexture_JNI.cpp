//
//  AnimatedTexture_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <memory>
#include <VROTextureUtil.h>
#include <VROAnimatedTextureOpenGL.h>
#include "VRONode.h"
#include "VROMaterial.h"
#include "VROFrameSynchronizer.h"
#include "VROPlatformUtil.h"
#include "ViroContext_JNI.h"
#include "VRODriverOpenGL.h"
#include "Quad_JNI.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_AnimatedTexture_##method_name

extern "C" {

VRO_METHOD(VRO_REF(VROAnimatedTextureOpenGL), nativeCreateAnimatedTexture)() {
    VRO_METHOD_PREAMBLE;
    std::shared_ptr<VROAnimatedTextureOpenGL> animTexture = std::make_shared<VROAnimatedTextureOpenGL>();
    return VRO_REF_NEW(VROAnimatedTextureOpenGL, animTexture);
}

VRO_METHOD(void, nativeDeleteAnimatedTexture)(VRO_ARGS
                                              VRO_REF(VROVideoTexture) textureRef) {
    VRO_REF_DELETE(VROAnimatedTextureOpenGL, textureRef);
}


VRO_METHOD(void, nativeLoadSource)(VRO_ARGS
                                   VRO_REF(VROAnimatedTexture) textureRef,
                                   VRO_STRING source,
                                   VRO_OBJECT jTexture,
                                   VRO_REF(ViroContext) context_j) {
    VRO_METHOD_PREAMBLE;

    // Grab required objects from the RenderContext required for initialization
    std::string strAnimatedTextureSrc = VRO_STRING_STL(source);
    std::weak_ptr<VROAnimatedTextureOpenGL> animatedTexture_w = VRO_REF_GET(VROAnimatedTextureOpenGL, textureRef);
    std::weak_ptr<ViroContext> context_w = VRO_REF_GET(ViroContext, context_j);
    VRO_WEAK weakJTexture = VRO_NEW_WEAK_GLOBAL_REF(jTexture);

    // Construct callback to be triggered over JNI upon initialization completion.
    std::function<void(bool, std::string)> finishCallback =
            [weakJTexture](bool succeeded, std::string msg) {
                VROPlatformDispatchAsyncApplication([weakJTexture, succeeded, msg] {
                    VRO_ENV env = VROPlatformGetJNIEnv();
                    VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakJTexture);
                    if (VRO_IS_OBJECT_NULL(localObj)) {
                        VRO_DELETE_WEAK_GLOBAL_REF(weakJTexture);
                        return;
                    }

                    VRO_STRING jerror = VRO_NEW_STRING(msg.c_str());
                    VROPlatformCallHostFunction(localObj, "onSourceLoaded", "(ZLjava/lang/String;)V", succeeded, jerror);
                    VRO_DELETE_LOCAL_REF(localObj);
                    VRO_DELETE_LOCAL_REF(jerror);
                    VRO_DELETE_WEAK_GLOBAL_REF(weakJTexture);
                });

            };

    // Finally load up the animated texture.
    VROPlatformDispatchAsyncRenderer([animatedTexture_w, context_w, finishCallback, strAnimatedTextureSrc] {
        std::shared_ptr<VROAnimatedTextureOpenGL> animatedTexture = animatedTexture_w.lock();
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!animatedTexture || !context) {
            finishCallback(false, "VROAnimatedTextureOpenGL has been destroyed.");
            return;
        }

        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer = context->getFrameSynchronizer();
        std::shared_ptr<VRODriver> driver = context->getDriver();
        animatedTexture->loadAnimatedSourceAsync(strAnimatedTextureSrc, context->getDriver(), frameSynchronizer, finishCallback);
    });
}

VRO_METHOD(void, nativePause)(VRO_ARGS
                              VRO_REF(VROAnimatedTexture) textureRef) {
    std::weak_ptr<VROAnimatedTextureOpenGL> animatedTexture_w = VRO_REF_GET(VROAnimatedTextureOpenGL, textureRef);
    VROPlatformDispatchAsyncRenderer([animatedTexture_w] {
        std::shared_ptr<VROAnimatedTextureOpenGL> animatedTexture = animatedTexture_w.lock();
        if (!animatedTexture) {
            return;
        }
        animatedTexture->pause();
    });
}

VRO_METHOD(void, nativePlay)(VRO_ARGS
                             VRO_REF(VROAnimatedTexture) textureRef) {
    std::weak_ptr<VROAnimatedTextureOpenGL> animatedTexture_w = VRO_REF_GET(VROAnimatedTextureOpenGL, textureRef);
    VROPlatformDispatchAsyncRenderer([animatedTexture_w] {
        std::shared_ptr<VROAnimatedTextureOpenGL> animatedTexture = animatedTexture_w.lock();
        if (!animatedTexture) {
            return;
        }
        animatedTexture->play();
    });
}

VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                VRO_REF(VROAnimatedTexture) textureRef,
                                VRO_BOOL loop) {
    std::weak_ptr<VROAnimatedTextureOpenGL> animatedTexture_w = VRO_REF_GET(VROAnimatedTextureOpenGL, textureRef);
    VROPlatformDispatchAsyncRenderer([animatedTexture_w, loop] {
        std::shared_ptr<VROAnimatedTextureOpenGL> animatedTexture = animatedTexture_w.lock();
        if (!animatedTexture) {
            return;
        }
        animatedTexture->setLoop(loop);
    });
}

}  // extern "C"