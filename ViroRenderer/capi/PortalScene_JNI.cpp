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
#else
#define VRO_METHOD(return_type, method_name) \
    return_type PortalScene_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROPortal), nativeCreatePortalScene)(VRO_NO_ARGS) {
    std::shared_ptr<VROPortal> portal = std::make_shared<VROPortal>();
    return VRO_REF_NEW(VROPortal, portal);
}

VRO_METHOD(void, nativeDestroyPortalScene)(VRO_ARGS
                                           VRO_REF(VROPortal) portalRef) {
    VRO_REF_DELETE(VROPortal, portalRef);
}

VRO_METHOD(void, nativeSetPassable)(VRO_ARGS
                                    VRO_REF(VROPortal) nativeRef, VRO_BOOL passable) {
    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, nativeRef);
    VROPlatformDispatchAsyncRenderer([portal_w, passable] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (portal) {
            portal->setPassable(passable);
        }
    });
}

VRO_METHOD(VRO_REF(PortalDelegate), nativeCreatePortalDelegate)(VRO_NO_ARGS) {
    VRO_METHOD_PREAMBLE;
    VROPlatformSetEnv(env);
    std::shared_ptr<PortalDelegate> delegate = std::make_shared<PortalDelegate>(obj);
    return VRO_REF_NEW(PortalDelegate, delegate);
}

VRO_METHOD(void, nativeDestroyPortalDelegate)(VRO_ARGS
                                              VRO_REF(PortalDelegate) delegateRef) {
    VRO_REF_DELETE(PortalDelegate, delegateRef);
}


VRO_METHOD(void, nativeAttachDelegate)(VRO_ARGS
                                       VRO_REF(VROPortal) portalRef,
                                       VRO_REF(PortalDelegate) delegateRef) {

    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, portalRef);
    std::weak_ptr<PortalDelegate> portalDelegate_w = VRO_REF_GET(PortalDelegate, delegateRef);

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
                                          VRO_REF(VROPortal) portalSceneRef,
                                          VRO_REF(VROPortalFrame) portalRef) {
    std::weak_ptr<VROPortal> portalScene_w = VRO_REF_GET(VROPortal, portalSceneRef);
    std::weak_ptr<VROPortalFrame> portalFrame_w = VRO_REF_GET(VROPortalFrame, portalRef);
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
                                             VRO_REF(VROPortal) portal_j,
                                             VRO_REF(VROTexture) texture_j) {
    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, portal_j);
    std::weak_ptr<VROTexture> texture_w = VRO_REF_GET(VROTexture, texture_j);

    VROPlatformDispatchAsyncRenderer([portal_w, texture_w] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();

        if (portal && texture) {
            portal->setBackgroundSphere(texture);
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundRotation)(VRO_ARGS
                                              VRO_REF(VROPortal) portal_j,
                                              VRO_FLOAT rotationRadiansX,
                                              VRO_FLOAT rotationRadiansY,
                                              VRO_FLOAT rotationRadiansZ) {
    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, portal_j);

    VROPlatformDispatchAsyncRenderer([portal_w, rotationRadiansX, rotationRadiansY, rotationRadiansZ] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (portal) {
            portal->setBackgroundRotation({rotationRadiansX, rotationRadiansY, rotationRadiansZ});
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundCubeImageTexture)(VRO_ARGS
                                                      VRO_REF(VROPortal) portal_j,
                                                      VRO_REF(VROTexture) texture_j) {
    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, portal_j);
    std::weak_ptr<VROTexture> texture_w = VRO_REF_GET(VROTexture, texture_j);

    VROPlatformDispatchAsyncRenderer([portal_w, texture_w] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();
        if (portal && texture) {
            portal->setBackgroundCube(texture);
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundCubeWithColor)(VRO_ARGS
                                                   VRO_REF(VROPortal) portal_j,
                                                   VRO_LONG color) {
    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, portal_j);
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
                                               VRO_REF(VROPortal) portal_j,
                                               VRO_REF(VROTexture) texture_j) {
    std::weak_ptr<VROPortal> portal_w = VRO_REF_GET(VROPortal, portal_j);

    VROPlatformDispatchAsyncRenderer([portal_w, texture_j] {
        std::shared_ptr<VROPortal> portal = portal_w.lock();
        if (!portal) {
            return;
        }

        if (texture_j == 0) {
            portal->setLightingEnvironment(nullptr);
            return;
        }

        std::shared_ptr<VROTexture> texture = VRO_REF_GET(VROTexture, texture_j);
        if (texture) {
            portal->setLightingEnvironment(texture);
        }
    });
}

}