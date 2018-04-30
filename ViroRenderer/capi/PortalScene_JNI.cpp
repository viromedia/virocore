/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include "Portal_JNI.h"
#include "PortalScene_JNI.h"
#include "PortalDelegate_JNI.h"
#include "Texture_JNI.h"
#include <VROPlatformUtil.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_PortalScene_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreatePortalScene)(VRO_NO_ARGS) {
    std::shared_ptr<VROPortal> portal = std::make_shared<VROPortal>();
    return PortalScene::jptr(portal);
}

VRO_METHOD(void, nativeDestroyPortalScene)(VRO_ARGS
                                           VRO_REF portalRef) {
    delete reinterpret_cast<PersistentRef<VROPortal> *>(portalRef);
}

VRO_METHOD(void, nativeSetPassable)(VRO_ARGS
                                    VRO_REF nativeRef, VRO_BOOL passable) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([portal_w, passable] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (portal) {
            portal->setPassable(passable);
        }
    });
}

VRO_METHOD(VRO_REF, nativeCreatePortalDelegate)(VRO_NO_ARGS) {
    VROPlatformSetEnv(env);
    std::shared_ptr<PortalDelegate> delegate = std::make_shared<PortalDelegate>(obj);
    return PortalDelegate::jptr(delegate);
}

VRO_METHOD(void, nativeDestroyPortalDelegate)(VRO_ARGS
                                              VRO_REF delegateRef) {
    delete reinterpret_cast<PersistentRef<PortalDelegate> *>(delegateRef);
}


VRO_METHOD(void, nativeAttachDelegate)(VRO_ARGS
                                       VRO_REF portalRef, VRO_REF delegateRef) {

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

VRO_METHOD(void, nativeSetPortalEntrance)(VRO_ARGS
                                          VRO_REF portalSceneRef,
                                          VRO_REF portalRef) {
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

VRO_METHOD(void, nativeSetBackgroundTexture)(VRO_ARGS
                                             VRO_REF portal_j,
                                             VRO_REF texture_j) {
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

VRO_METHOD(void, nativeSetBackgroundRotation)(VRO_ARGS
                                              VRO_REF portal_j,
                                              VRO_FLOAT rotationRadiansX,
                                              VRO_FLOAT rotationRadiansY,
                                              VRO_FLOAT rotationRadiansZ) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portal_j);

    VROPlatformDispatchAsyncRenderer([portal_w, rotationRadiansX, rotationRadiansY, rotationRadiansZ] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (portal) {
            portal->setBackgroundRotation({rotationRadiansX, rotationRadiansY, rotationRadiansZ});
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundCubeImageTexture)(VRO_ARGS
                                                      VRO_REF portal_j,
                                                      VRO_REF texture_j) {
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

VRO_METHOD(void, nativeSetBackgroundCubeWithColor)(VRO_ARGS
                                                   VRO_REF portal_j,
                                                   VRO_LONG color) {
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

VRO_METHOD(void, nativeSetLightingEnvironment)(VRO_ARGS
                                               VRO_REF portal_j,
                                               VRO_REF texture_j) {
    std::weak_ptr<VROPortal> portal_w = PortalScene::native(portal_j);
    long texture_ref = texture_j;
    VROPlatformDispatchAsyncRenderer([portal_w, texture_ref] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (!portal){
            return;
        }

        if (texture_ref != 0){
            portal->setLightingEnvironment(Texture::native(texture_ref));
        } else {
            portal->setLightingEnvironment(nullptr);
        }
    });
}

}