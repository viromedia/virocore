//
//  ParticleEmitter_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ParticleEmitter_JNI.h"
#include "ViroContext_JNI.h"
#include "Node_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ParticleEmitter_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateEmitter)(VRO_ARGS
                                         VRO_REF context_j,
                                         VRO_REF native_surface_ref) {
    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VROSurface> surface = reinterpret_cast<PersistentRef<VROSurface> *>(native_surface_ref)->get();
    std::shared_ptr<VROParticleEmitter> particleEmitter = std::make_shared<VROParticleEmitter>();

    VROPlatformDispatchAsyncRenderer([particleEmitter, context, surface] {
        particleEmitter->initEmitter(context->getDriver(), surface);
    });

    return ParticleEmitter::jptr(particleEmitter);
}

VRO_METHOD(void, nativeDestroyEmitter)(VRO_ARGS
                                       VRO_REF nativeParticleEmitterRef) {
    delete reinterpret_cast<PersistentRef<VROParticleEmitter> *>(nativeParticleEmitterRef);
}

VRO_METHOD(void, nativeSetDelay)(VRO_ARGS
                                 VRO_REF native_ref,
                                 VRO_FLOAT delay) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, delay] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setDelay(delay);
        }
    });
}

VRO_METHOD(void, nativeSetDuration)(VRO_ARGS
                                    VRO_REF native_ref,
                                    VRO_FLOAT value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setDuration(value);
        }
    });
}

VRO_METHOD(void, nativeSetLoop)(VRO_ARGS
                                VRO_REF native_ref,
                                jboolean value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setLoop(value);
        }
    });
}

VRO_METHOD(void, nativeSetRun)(VRO_ARGS
                               VRO_REF native_ref,
                               jboolean value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setRun(value);
        }
    });
}

VRO_METHOD(void, nativeSetFixedToEmitter)(VRO_ARGS
                                          VRO_REF native_ref,
                                          jboolean value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setFixedToEmitter(value);
        }
    });
}

VRO_METHOD(void, nativeResetEmissionCycle)(VRO_ARGS
                                           VRO_REF native_ref) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->resetEmissionCycle(true);
        }
    });
}


VRO_METHOD(void, nativeSetEmissionRatePerSecond)(VRO_ARGS
                                                 VRO_REF native_ref,
                                                 jint value1, jint value2) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1, value2] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setEmissionRatePerSecond(std::pair<int, int>(value1, value2));
        }
    });
}

VRO_METHOD(void, nativeSetEmissionRatePerMeter)(VRO_ARGS
                                                VRO_REF native_ref,
                                                jint value1, jint value2) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1, value2] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setEmissionRatePerDistance(std::pair<int, int>(value1, value2));
        }
    });
}

VRO_METHOD(void, nativeSetParticleLifetime)(VRO_ARGS
                                            VRO_REF native_ref,
                                            jint value1, jint value2) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1, value2] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setParticleLifeTime(std::pair<int, int>(value1, value2));
        }
    });
}

VRO_METHOD(void, nativeSetMaxParticles)(VRO_ARGS
                                        VRO_REF native_ref,
                                        jint value1) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setMaxParticles(value1);
        }
    });
}

VRO_METHOD(void, nativeSetSpawnVolume)(VRO_ARGS
                                       VRO_REF native_ref,
                                       VRO_STRING jShape,
                                       VRO_FLOAT_ARRAY jShapeParams,
                                       jboolean jSpawnOnSurface) {

    // Grab the emitter's spawn volume shape.
    std::string strShape = VROPlatformGetString(jShape, env);
    VROParticleSpawnVolume::Shape shape
            = VROParticleSpawnVolume::getModifierFactorForString(strShape);

    // Also grab any parameters describing the shape if any.
    std::vector<float> params;
    if (jShapeParams != NULL) {
        int paramsLength = VRO_ARRAY_LENGTH(jShapeParams);
        VRO_FLOAT *pointArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(jShapeParams);
        for (int i = 0; i < paramsLength; i ++) {
            params.push_back(pointArray[i]);
        }
        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(jShapeParams, pointArray);
    }

    // Finally create and set the emitter's spawn volume.
    VROParticleSpawnVolume spawnVolume;
    spawnVolume.shape = shape;
    spawnVolume.shapeParams = params;
    spawnVolume.spawnOnSurface = jSpawnOnSurface;

    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, spawnVolume] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setParticleSpawnVolume(spawnVolume);
        }
    });
}

VRO_METHOD(void, nativeSetExplosiveImpulse)(VRO_ARGS
                                            VRO_REF native_ref,
                                            VRO_FLOAT jImpulse,
                                            VRO_FLOAT_ARRAY jPosition,
                                            VRO_FLOAT jDeccelPeriod) {
    // Grab the position at which to apply the explosive impulse.
    int paramsLength = VRO_ARRAY_LENGTH(jPosition);
    VRO_FLOAT *pointArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(jPosition);
    std::vector<float> position;
    for (int i = 0; i < paramsLength; i ++) {
        position.push_back(pointArray[i]);
    }
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(jPosition, pointArray);
    VROVector3f vecPos = VROVector3f(position[0], position[1], position[2]);

    // Apply the explosive impulse on the emitter.
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, vecPos, jImpulse, jDeccelPeriod] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setInitialExplosion(vecPos, jImpulse, jDeccelPeriod);
        }
    });
}

VRO_METHOD(void, nativeSetParticleBursts)(VRO_ARGS
                                          VRO_REF native_ref,
                                          jobjectArray jBursts) {

    // Grab and create a list of bursts if any.
    std::vector<VROParticleEmitter::VROParticleBurst> particleBursts;
    if (jBursts != NULL) {
        int numberOfValues = VRO_ARRAY_LENGTH(jBursts);
        for (int i = 0; i < numberOfValues; i++) {
            jdoubleArray burstData = (jdoubleArray)env->GetObjectArrayElement(jBursts, i);
            jdouble *burstDataArray = env->GetDoubleArrayElements(burstData, 0);

            VROParticleModifier::VROModifierFactor factor
                    = burstDataArray[0] == 1 ?
                      VROParticleModifier::VROModifierFactor::Time
                                             : VROParticleModifier::VROModifierFactor::Distance;
            jdouble start = burstDataArray[1];
            jdouble min = burstDataArray[2];
            jdouble max = burstDataArray[3];
            jdouble period = burstDataArray[4];
            jdouble cycles = burstDataArray[5];

            VROParticleEmitter::VROParticleBurst burst;
            burst.referenceValueStart = start;
            burst.numberOfParticles = std::pair<int, int>(min, max);
            burst.referenceValueInterval = period;
            burst.cycles = cycles;
            burst.referenceFactor = factor;
            particleBursts.push_back(burst);
        }
    }

    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, particleBursts] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setParticleBursts(particleBursts);
        }
    });
}

VRO_METHOD(void, nativeSetParticleModifier)(VRO_ARGS
                                            VRO_REF native_ref,
                                            VRO_STRING jModifier,
                                            VRO_STRING jFactor,
                                            jobjectArray jInitialValues,
                                            jobjectArray jInterpolatedIntervalWindows,
                                            jobjectArray jInterpolatedPoints) {

    // Construct a modifier with the given initialValues and interpolation values.
    std::shared_ptr<VROParticleModifier> mod = ParticleEmitter::getParticleModifier(env, jFactor,
                                                                                    jInitialValues,
                                                                                    jInterpolatedIntervalWindows,
                                                                                    jInterpolatedPoints);
    std::string strModifier = VROPlatformGetString(jModifier, env);

    // Apply the modifier on the targeted property, like opacity.
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, mod, strModifier] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (!emitter) {
            return;
        }

        if (VROStringUtil::strcmpinsensitive(strModifier, "opacity")) {
            emitter->setAlphaModifier(mod);
        } else if (VROStringUtil::strcmpinsensitive(strModifier, "scale")) {
            emitter->setScaleModifier(mod);
        } else if (VROStringUtil::strcmpinsensitive(strModifier, "rotation")) {
            emitter->setRotationModifier(mod);
        } else if (VROStringUtil::strcmpinsensitive(strModifier, "color")) {
            emitter->setColorModifier(mod);
        } else if (VROStringUtil::strcmpinsensitive(strModifier, "velocity")) {
            emitter->setVelocityModifier(mod);
        } else if (VROStringUtil::strcmpinsensitive(strModifier, "acceleration")) {
            emitter->setAccelerationmodifier(mod);
        } else {
            perror("Viro: Internal Error - attempted to configure invalid modifier!");
        }
    });
}

VRO_METHOD(bool, nativeSetParticleBlendMode)(VRO_ARGS
                                             VRO_REF native_ref,
                                             VRO_STRING jblendMode) {
    std::string strBlendMode = VROPlatformGetString(jblendMode, env);
    VROBlendMode mode = VROMaterial::getBlendModeFromString(strBlendMode);
    if (mode == VROBlendMode::None){
        pwarn("Viro: Attempted to set invalid Blend mode %s", strBlendMode.c_str());
        return false;
    }

    // Apply the modifier on the targeted property, like opacity.
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, mode] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (!emitter) {
            return;
        }

        emitter->setBlendMode(mode);
    });
    return true;
}

VRO_METHOD(void, nativeSetBloomThreshold)(VRO_ARGS
                                          VRO_REF native_ref,
                                          VRO_FLOAT threshold) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, threshold] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setBloomThreshold(threshold);
        }
    });
}

}  // extern "C"
