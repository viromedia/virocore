//
//  ParticleEmitter_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ParticleEmitter_JNI.h"
#include "ViroContext_JNI.h"
#include "Node_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ParticleEmitter_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateEmitter)(JNIEnv *env,
                                        jobject obj,
                                   jlong context_j,
                                   jlong native_node_ref,
                                   jlong native_surface_ref) {
    std::shared_ptr<ViroContext> context = ViroContext::native(context_j);
    std::shared_ptr<VRONode> node = Node::native(native_node_ref);
    std::shared_ptr<VROSurface> surface = reinterpret_cast<PersistentRef<VROSurface> *>(native_surface_ref)->get();
    std::shared_ptr<VROParticleEmitter> particleEmitter = std::make_shared<VROParticleEmitter>();

    VROPlatformDispatchAsyncRenderer([particleEmitter, context, node, surface] {
        particleEmitter->initEmitter(context->getDriver(), node, surface);
    });

    return ParticleEmitter::jptr(particleEmitter);
}

JNI_METHOD(void, nativeDestroyEmitter)(JNIEnv *env,
                                        jclass clazz,
                                        jlong nativeParticleEmitterRef) {
    delete reinterpret_cast<PersistentRef<VROParticleEmitter> *>(nativeParticleEmitterRef);
}

JNI_METHOD(void, nativeSetDelay)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_ref,
                                 jfloat delay) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, delay] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setDelay(delay);
        }
    });
}

JNI_METHOD(void, nativeSetDuration)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_ref,
                                 jfloat value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setDuration(value);
        }
    });
}

JNI_METHOD(void, nativeSetLoop)(JNIEnv *env,
                                    jclass clazz,
                                    jlong native_ref,
                                    jboolean value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setLoop(value);
        }
    });
}

JNI_METHOD(void, nativeSetRun)(JNIEnv *env,
                                jclass clazz,
                                jlong native_ref,
                                jboolean value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setRun(value);
        }
    });
}

JNI_METHOD(void, nativeSetFixedToEmitter)(JNIEnv *env,
                                jclass clazz,
                                jlong native_ref,
                                jboolean value) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setFixedToEmitter(value);
        }
    });
}

JNI_METHOD(void, nativeResetEmissionCycle)(JNIEnv *env,
                                jclass clazz,
                                jlong native_ref) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->resetEmissionCycle(true);
        }
    });
}


JNI_METHOD(void, nativeSetEmissionRatePerSecond)(JNIEnv *env,
                                 jclass clazz,
                                 jlong native_ref,
                                 jint value1, jint value2) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1, value2] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setEmissionRatePerSecond(std::pair<int, int>(value1, value2));
        }
    });
}

JNI_METHOD(void, nativeSetEmissionRatePerMeter)(JNIEnv *env,
                                                 jclass clazz,
                                                 jlong native_ref,
                                                 jint value1, jint value2) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1, value2] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setEmissionRatePerDistance(std::pair<int, int>(value1, value2));
        }
    });
}

JNI_METHOD(void, nativeSetParticleLifetime)(JNIEnv *env,
                                                 jclass clazz,
                                                 jlong native_ref,
                                                 jint value1, jint value2) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1, value2] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setParticleLifeTime(std::pair<int, int>(value1, value2));
        }
    });
}

JNI_METHOD(void, nativeSetMaxParticles)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_ref,
                                        jint value1) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, value1] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setMaxParticles(value1);
        }
    });
}

JNI_METHOD(void, nativeSetSpawnVolume)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_ref,
                                        jstring jShape,
                                        jfloatArray jShapeParams,
                                        jboolean jSpawnOnSurface) {

    // Grab the emitter's spawn volume shape.
    std::string strShape = VROPlatformGetString(jShape);
    VROParticleSpawnVolume::Shape shape
            = VROParticleSpawnVolume::getModifierFactorForString(strShape);

    // Also grab any parameters describing the shape if any.
    std::vector<float> params;
    if (jShapeParams != NULL) {
        int paramsLength = env->GetArrayLength(jShapeParams);
        jfloat *pointArray = env->GetFloatArrayElements(jShapeParams, 0);
        for (int i = 0; i < paramsLength; i ++) {
            params.push_back(pointArray[i]);
        }
        env->ReleaseFloatArrayElements(jShapeParams, pointArray, 0);
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

JNI_METHOD(void, nativeSetExplosiveImpulse)(JNIEnv *env,
                                       jclass clazz,
                                       jlong native_ref,
                                       jfloat jImpulse,
                                       jfloatArray jPosition,
                                       jfloat jDeccelPeriod) {
    // Grab the position at which to apply the explosive impulse.
    int paramsLength = env->GetArrayLength(jPosition);
    jfloat *pointArray = env->GetFloatArrayElements(jPosition, 0);
    std::vector<float> position;
    for (int i = 0; i < paramsLength; i ++) {
        position.push_back(pointArray[i]);
    }
    env->ReleaseFloatArrayElements(jPosition, pointArray, 0);
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

JNI_METHOD(void, nativeSetParticleBursts)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_ref,
                                        jobjectArray jBursts) {

    // Grab and create a list of bursts if any.
    std::vector<VROParticleEmitter::VROParticleBurst> particleBursts;
    if (jBursts != NULL) {
        int numberOfValues = env->GetArrayLength(jBursts);
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

JNI_METHOD(void, nativeSetParticleModifier)(JNIEnv *env,
                                            jclass clazz,
                                            jlong native_ref,
                                            jstring jModifier,
                                            jstring jFactor,
                                            jobjectArray jInitialValues,
                                            jobjectArray jInterpolatedIntervalWindows,
                                            jobjectArray jInterpolatedPoints) {

    // Construct a modifier with the given initialValues and interpolation values.
    std::shared_ptr<VROParticleModifier> mod = ParticleEmitter::getParticleModifier(env, jFactor,
                                                                                    jInitialValues,
                                                                                    jInterpolatedIntervalWindows,
                                                                                    jInterpolatedPoints);
    std::string strModifier = VROPlatformGetString(jModifier);

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

JNI_METHOD(bool, nativeSetParticleBlendMode)(JNIEnv *env,
                                             jclass clazz,
                                             jlong native_ref,
                                             jstring jblendMode) {
    std::string strBlendMode = VROPlatformGetString(jblendMode);
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

JNI_METHOD(void, nativeSetBloomThreshold)(JNIEnv *env,
                                                jclass clazz,
                                                jlong native_ref,
                                                jfloat threshold) {
    std::weak_ptr<VROParticleEmitter> native_w = ParticleEmitter::native(native_ref);
    VROPlatformDispatchAsyncRenderer([native_w, threshold] {
        std::shared_ptr<VROParticleEmitter> emitter = native_w.lock();
        if (emitter) {
            emitter->setBloomThreshold(threshold);
        }
    });
}

}  // extern "C"
