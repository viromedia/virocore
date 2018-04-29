//
// Created by Raj Advani on 10/31/17.
//

#include "AnimationTransaction_JNI.h"
#include "VROPlatformUtil.h"
#include "VROStringUtil.h"
#include "VROLog.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
        Java_com_viro_core_AnimationTransaction_##method_name
#endif

extern "C" {

VRO_METHOD(long, nativeBegin)(VRO_NO_ARGS_STATIC) {
    std::shared_ptr<VROTransaction> transaction = std::shared_ptr<VROTransaction>(new VROTransaction());
    VROPlatformDispatchAsyncRenderer([transaction] {
        VROTransaction::add(transaction);
    });
    return AnimationTransaction::jptr(transaction);
}

VRO_METHOD(void, nativeCommit)(VRO_ARGS_STATIC
                               VRO_OBJECT obj) {
    VRO_OBJECT jGlobalObj = VRO_NEW_GLOBAL_REF(obj);
    VROPlatformDispatchAsyncRenderer([jGlobalObj] {

        VROTransaction::setFinishCallback([jGlobalObj](bool terminate) {

            VROPlatformDispatchAsyncApplication([jGlobalObj, terminate] {
                JNIEnv *env = VROPlatformGetJNIEnv();
                VROPlatformCallJavaFunction(jGlobalObj, "onAnimationFinished", "()V");
                if (terminate) {
                    VRO_DELETE_GLOBAL_REF(jGlobalObj);
                }
            });
        });
        VROTransaction::commit();
    });
}

VRO_METHOD(void, nativeSetAnimationDelay)(VRO_ARGS_STATIC
                                          float delay) {
    VROPlatformDispatchAsyncRenderer([delay] {
        VROTransaction::setAnimationDelay(delay);
    });
}

VRO_METHOD(void, nativeSetAnimationDuration)(VRO_ARGS_STATIC
                                             float duration) {
    VROPlatformDispatchAsyncRenderer([duration] {
        VROTransaction::setAnimationDuration(duration);
    });
}

VRO_METHOD(void, nativeSetAnimationLoop)(VRO_ARGS_STATIC
                                         jboolean loop) {
    VROPlatformDispatchAsyncRenderer([loop] {
        VROTransaction::setAnimationLoop(loop);
    });
}

VRO_METHOD(void, nativeSetTimingFunction)(VRO_ARGS_STATIC
                                          VRO_STRING timing_j) {
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

VRO_METHOD(void, nativeDispose)(VRO_ARGS
                                VRO_REF transaction_j) {
    delete reinterpret_cast<PersistentRef<VROTransaction> *>(transaction_j);
}

VRO_METHOD(void, nativePause)(VRO_ARGS
                              VRO_REF transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = AnimationTransaction::native(transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::pause(transaction);
    });
}

VRO_METHOD(void, nativeResume)(VRO_ARGS
                               VRO_REF transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = AnimationTransaction::native(transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::resume(transaction);
    });
}

VRO_METHOD(void, nativeTerminate)(VRO_ARGS
                                  VRO_REF transaction_j) {
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
