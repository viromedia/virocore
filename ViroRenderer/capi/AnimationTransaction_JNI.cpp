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
#else
#define VRO_METHOD(return_type, method_name) \
    return_type AnimationTransaction_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROTransaction), nativeBegin)(VRO_NO_ARGS_STATIC) {
    std::shared_ptr<VROTransaction> transaction = std::shared_ptr<VROTransaction>(new VROTransaction());
    VROPlatformDispatchAsyncRenderer([transaction] {
        VROTransaction::add(transaction);
    });
    return VRO_REF_NEW(VROTransaction, transaction);
}

VRO_METHOD(void, nativeCommit)(VRO_ARGS_STATIC
                               VRO_OBJECT obj) {
    VRO_OBJECT jGlobalObj = VRO_NEW_GLOBAL_REF(obj);
    VROPlatformDispatchAsyncRenderer([jGlobalObj] {

        VROTransaction::setFinishCallback([jGlobalObj](bool terminate) {

            VROPlatformDispatchAsyncApplication([jGlobalObj, terminate] {
                VRO_ENV env = VROPlatformGetJNIEnv();
                VROPlatformCallHostFunction(jGlobalObj, "onAnimationFinished", "()V");
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

VRO_METHOD(void, nativeSetAnimationTimeOffset)(VRO_ARGS_STATIC float timeOffset) {
    VROPlatformDispatchAsyncRenderer([timeOffset] {
        VROTransaction::setAnimationTimeOffset(timeOffset);
    });
}

VRO_METHOD(void, nativeSetAnimationSpeed)(VRO_ARGS_STATIC float speed) {
    VROPlatformDispatchAsyncRenderer([speed] {
        VROTransaction::setAnimationSpeed(speed);
    });
}


VRO_METHOD(void, nativeSetAnimationDuration)(VRO_ARGS_STATIC
                                             float duration) {
    VROPlatformDispatchAsyncRenderer([duration] {
        VROTransaction::setAnimationDuration(duration);
    });
}

VRO_METHOD(void, nativeSetAnimationLoop)(VRO_ARGS_STATIC
                                         VRO_BOOL loop) {
    VROPlatformDispatchAsyncRenderer([loop] {
        VROTransaction::setAnimationLoop(loop);
    });
}

VRO_METHOD(void, nativeSetTimingFunction)(VRO_ARGS_STATIC
                                          VRO_STRING timing_j) {
    std::string timing_s = VRO_STRING_STL(timing_j);
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
                                VRO_REF(VROTransaction) transaction_j) {
    VRO_REF_DELETE(VROTransaction, transaction_j);
}

VRO_METHOD(void, nativePause)(VRO_ARGS
                              VRO_REF(VROTransaction) transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = VRO_REF_GET(VROTransaction, transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::pause(transaction);
    });
}

VRO_METHOD(void, nativeResume)(VRO_ARGS
                               VRO_REF(VROTransaction) transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = VRO_REF_GET(VROTransaction, transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::resume(transaction);
    });
}

VRO_METHOD(void, nativeTerminate)(VRO_ARGS
                                  VRO_REF(VROTransaction) transaction_j) {
    std::weak_ptr<VROTransaction> transaction_w = VRO_REF_GET(VROTransaction, transaction_j);
    VROPlatformDispatchAsyncRenderer([transaction_w] {
        std::shared_ptr<VROTransaction> transaction = transaction_w.lock();
        if (!transaction) {
            return;
        }
        VROTransaction::terminate(transaction, true);
    });
}

}
