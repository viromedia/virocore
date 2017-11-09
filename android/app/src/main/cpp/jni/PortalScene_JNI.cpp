/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include "Portal_JNI.h"
#include "PortalScene_JNI.h"
#include "PortalDelegate_JNI.h"
#include "Texture_JNI.h"
#include <VROPlatformUtil.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_PortalScene_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreatePortalScene)(JNIEnv *env,
                                           jclass clazz) {
    std::shared_ptr<VROPortal> portal = std::make_shared<VROPortal>();
    return PortalScene::jptr(portal);
}

JNI_METHOD(void, nativeDestroyPortalScene)(JNIEnv *env,
                                           jobject object,
                                           jlong portalRef) {
    delete reinterpret_cast<PersistentRef<VROPortal> *>(portalRef);
}

JNI_METHOD(void, nativeSetPassable)(JNIEnv *env,
                                     jobject object, jlong nativeRef, jboolean passable) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([portal_w, passable] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (portal) {
            portal->setPassable(passable);
        }
    });
}

JNI_METHOD(jlong, nativeCreatePortalDelegate)(JNIEnv *env,
                                              jobject object) {
    VROPlatformSetEnv(env);
    std::shared_ptr<PortalDelegate> delegate = std::make_shared<PortalDelegate>(object);
    return PortalDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeDestroyPortalDelegate)(JNIEnv *env,
                                              jobject object,
                                              jlong delegateRef) {
    delete reinterpret_cast<PersistentRef<PortalDelegate> *>(delegateRef);
}


JNI_METHOD(void, nativeAttachDelegate)(JNIEnv *env,
                                       jobject object,
                                       jlong portalRef, jlong delegateRef) {

    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portalRef);
    std::weak_ptr<PortalDelegate> portalDelegate_w = PortalDelegate::native(delegateRef);

    VROPlatformDispatchAsyncRenderer([portal_w, portalDelegate_w] {
        std::shared_ptr<VROPortal> vroPortal = portal_w.lock();
        if (!vroPortal) {
            return;
        }
        std::shared_ptr<PortalDelegate> portalDelegate = portalDelegate_w.lock();
        if (!portalDelegate) {
            return;
        }

        vroPortal->setPortalDelegate(portalDelegate);
    });
}

JNI_METHOD(void, nativeSetPortalEntrance)(JNIEnv *env, jclass clazz, jlong portalSceneRef,
                                    jlong portalRef) {
    std::weak_ptr<VROPortal> portalScene_w = PortalScene::native(portalSceneRef);
    std::weak_ptr<VROPortalFrame> portalFrame_w = Portal::native(portalRef);
    VROPlatformDispatchAsyncRenderer([portalScene_w, portalFrame_w] {
        std::shared_ptr<VROPortal> vroPortal = portalScene_w.lock();
        if (!vroPortal) {
            return;
        }
        std::shared_ptr<VROPortalFrame> portalFrame = portalFrame_w.lock();
        if (!portalFrame) {
            return;
        }

        vroPortal->setPortalEntrance(portalFrame);
    });
}

JNI_METHOD(void, nativeSetBackgroundTexture)(JNIEnv *env,
                                             jclass clazz,
                                             jlong portal_j,
                                             jlong texture_j) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portal_j);
    std::weak_ptr<VROTexture> texture_w = Texture::native(texture_j);

    VROPlatformDispatchAsyncRenderer([portal_w, texture_w] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();

        if (portal && texture) {
            portal->setBackgroundSphere(texture);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundRotation)(JNIEnv *env,
                                              jclass clazz,
                                              jlong portal_j,
                                              jfloat rotationRadiansX,
                                              jfloat rotationRadiansY,
                                              jfloat rotationRadiansZ) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portal_j);

    VROPlatformDispatchAsyncRenderer([portal_w, rotationRadiansX, rotationRadiansY, rotationRadiansZ] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (portal) {
            portal->setBackgroundRotation({rotationRadiansX, rotationRadiansY, rotationRadiansZ});
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundCubeImageTexture)(JNIEnv *env,
                                                      jclass clazz,
                                                      jlong portal_j,
                                                      jlong texture_j) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portal_j);
    std::weak_ptr<VROTexture> texture_w = Texture::native(texture_j);

    VROPlatformDispatchAsyncRenderer([portal_w, texture_w] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();
        if (portal && texture) {
            portal->setBackgroundCube(texture);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundCubeWithColor)(JNIEnv *env,
                                                   jclass clazz,
                                                   jlong portal_j,
                                                   jlong color) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portal_j);
    VROPlatformDispatchAsyncRenderer([portal_w, color] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (!portal) {
            return;
        }
        // Get the color
        float a = ((color >> 24) & 0xFF) / 255.0f;
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;

        VROVector4f vecColor(r, g, b, a);
        portal->setBackgroundCube(vecColor);
    });
}

}