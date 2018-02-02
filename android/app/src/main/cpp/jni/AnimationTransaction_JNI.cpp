//
// Created by Raj Advani on 10/31/17.
//

#include "AnimationTransaction_JNI.h"
#include "VROPlatformUtil.h"
#include "VROStringUtil.h"
#include "VROLog.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_AnimationTransaction_##method_name

extern "C" {

JNI_METHOD(long, nativeBegin)(JNIEnv *env, jclass clazz) {
    std::shared_ptr<VROTransaction> transaction = std::shared_ptr<VROTransaction>(new VROTransaction());
    VROPlatformDispatchAsyncRenderer([transaction] {
        VROTransaction::add(transaction);
    });
    return AnimationTransaction::jptr(transaction);
}

JNI_METHOD(void, nativeCommit)(JNIEnv *env, jclass clazz, jobject obj) {
    jobject jGlobalObj = env->NewGlobalRef(obj);
    VROPlatformDispatchAsyncRenderer([jGlobalObj] {

        VROTransaction::setFinishCallback([jGlobalObj](bool terminate) {

            VROPlatformDispatchAsyncApplication([jGlobalObj, terminate] {
                JNIEnv *env = VROPlatformGetJNIEnv();
                VROPlatformCallJavaFunction(jGlobalObj, "onAnimationFinished", "()V");
                if (terminate) {
                    env->DeleteGlobalRef(jGlobalObj);
                }
            });
        });
        VROTransaction::commit();
    });
}

JNI_METHOD(void, nativeSetAnimationDelay)(JNIEnv *env, jclass clazz, float delay) {
    VROPlatformDispatchAsyncRenderer([delay] {
        VROTransaction::setAnimationDelay(delay);
    });
}

JNI_METHOD(void, nativeSetAnimationDuration)(JNIEnv *env, jclass clazz, float duration) {
    VROPlatformDispatchAsyncRenderer([duration] {
        VROTransaction::setAnimationDuration(duration);
    });
}

JNI_METHOD(void, nativeSetAnimationLoop)(JNIEnv *env, jclass clazz, jboolean loop) {
    VROPlatformDispatchAsyncRenderer([loop] {
        VROTransaction::setAnimationLoop(loop);
    });
}

JNI_METHOD(void, nativeSetTimingFunction)(JNIEnv *env, jclass clazz, jstring timing_j) {
    std::string timing_s = VROPlatformGetString(timing_j, env);
    VROTimingFunctionType timing = VROTimingFunctionType::Linear;
    if (VROStringUtil::strcmpinsensitive(timing_s, "easein")) {
        timing = VROTimingFunctionType::EaseIn;
    } else if (VROStringUtil::strcmpinsensitive(timing_s, "easeout")) {
        timing = VROTimingFunctionType::EaseOut;
    } else if (VROStringUtil::strcmpinsensitive(timing_s, "easeineaseout")) {
        timing = VROTimingFunctionType::EaseInEaseOut;
    } else if (VROStringUtil::strcmpinsensitive(timing_s, "bounce")) {
        timing = VROTimingFunctionType::Bounce;
    }

    VROPlatformDispatchAsyncRenderer([timing] {
        VROTransaction::setTimingFunction(timing);
    });
}

JNI_METHOD(void, nativeDispose)(JNIEnv *env, jobject obj, jlong transaction_j) {
    delete reinterpret_cast<PersistentRef<VROTransaction> *>(transaction_j);
}

JNI_METHOD(void, nativePause)(JNIEnv *env, jobject obj, jlong transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = AnimationTransaction::native(transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::pause(transaction);
    });
}

JNI_METHOD(void, nativeResume)(JNIEnv *env, jobject obj, jlong transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = AnimationTransaction::native(transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::resume(transaction);
    });
}

JNI_METHOD(void, nativeTerminate)(JNIEnv *env, jobject obj, jlong transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = AnimationTransaction::native(transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::terminate(transaction, true);
    });
}

}
