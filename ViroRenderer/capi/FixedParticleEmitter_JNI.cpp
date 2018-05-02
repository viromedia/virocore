//
//  FixedParticleEmitter_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include <VROFixedParticleEmitter.h>
#include "FixedParticleEmitter_JNI.h"
#include "ARUtils_JNI.h"
#include "ViroContext_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_FixedParticleEmitter_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type FixedParticleEmitter_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROFixedParticleEmitter), nativeCreateEmitter)(VRO_ARGS
                                                                  VRO_REF(ViroContext) context_j,
                                                                  VRO_REF(VROSurface) native_surface_ref) {
    std::shared_ptr<VROFixedParticleEmitter> particleEmitter = std::make_shared<VROFixedParticleEmitter>();
    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VROSurface> surface = nullptr;
    if (native_surface_ref != 0){
        surface = reinterpret_cast<PersistentRef<VROSurface> *>(native_surface_ref)->get();
    }

    VROPlatformDispatchAsyncRenderer([particleEmitter, context, surface] {
        particleEmitter->initEmitter(context->getDriver(), surface);
    });

    return FixedParticleEmitter::jptr(particleEmitter);
}

VRO_METHOD(void, nativeDestroyEmitter)(VRO_ARGS
                                       VRO_REF(VROFixedParticleEmitter) nativeParticleEmitterRef) {
    delete reinterpret_cast<PersistentRef<VROFixedParticleEmitter> *>(nativeParticleEmitterRef);
}

VRO_METHOD(void, nativeSetParticles)(VRO_ARGS
                                     VRO_REF(VROFixedParticleEmitter) emitter_j,
                                     VRO_ARRAY(VRO_FLOAT_ARRAY) jPositions) {
    std::vector<VROVector4f> initialValues;
    int numberOfValues = VRO_ARRAY_LENGTH(jPositions);
    for (int i = 0; i < numberOfValues; i++) {
        VRO_FLOAT_ARRAY vec3Value = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(jPositions, i);
        VRO_FLOAT *vec3ValueArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(vec3Value);
        VROVector4f vec4 = VROVector4f(vec3ValueArray[0], vec3ValueArray[1], vec3ValueArray[2], 1.0);
        initialValues.push_back(vec4);
        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(vec3Value, vec3ValueArray);
    }

    std::weak_ptr<VROFixedParticleEmitter> emitter_w = FixedParticleEmitter::native(emitter_j);
    VROPlatformDispatchAsyncRenderer([emitter_w, initialValues] {
        std::shared_ptr<VROFixedParticleEmitter> emitter = emitter_w.lock();
        emitter->setParticleTransforms(initialValues);
    });
}

VRO_METHOD(void, nativeClearParticles)(VRO_ARGS
                                       VRO_REF(VROFixedParticleEmitter) emitter_j) {
    std::weak_ptr<VROFixedParticleEmitter> emitter_w = FixedParticleEmitter::native(emitter_j);
    VROPlatformDispatchAsyncRenderer([emitter_w] {
        std::shared_ptr<VROFixedParticleEmitter> emitter = emitter_w.lock();
        emitter->forceClearParticles();
    });
}

VRO_METHOD(void, nativeSetEmitterSurface)(VRO_ARGS
                                          VRO_REF(VROFixedParticleEmitter) emitter_j,
                                          VRO_REF(VROSurface) native_surface_ref) {
    std::weak_ptr<VROFixedParticleEmitter> emitter_w = FixedParticleEmitter::native(emitter_j);
    std::weak_ptr<VROSurface> surface_w = reinterpret_cast<PersistentRef<VROSurface> *>(native_surface_ref)->get();

    VROPlatformDispatchAsyncRenderer([emitter_w, surface_w] {
        std::shared_ptr<VROFixedParticleEmitter> emitter = emitter_w.lock();
        std::shared_ptr<VROSurface> surface = surface_w.lock();
        if (!surface || ! emitter){
            return;
        }

        emitter->setParticleSurface(surface);
    });
}

}
