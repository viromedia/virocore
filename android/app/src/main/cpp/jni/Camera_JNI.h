//
//  Camera_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_CAMERA_JNI_H
#define ANDROID_CAMERA_JNI_H

#include <jni.h>
#include <memory>
#include <VRONodeCamera.h>
#include <VROCamera.h>
#include <VROPlatformUtil.h>
#include <VROTime.h>
#include "PersistentRef.h"
#include "ARUtils_JNI.h"

namespace Camera {
    inline jlong jptr(std::shared_ptr<VRONodeCamera> shared_camera) {
        PersistentRef<VRONodeCamera> *native_camera = new PersistentRef<VRONodeCamera>(shared_camera);
        return reinterpret_cast<intptr_t>(native_camera);
    }

    inline std::shared_ptr<VRONodeCamera> native(jlong ptr) {
        PersistentRef<VRONodeCamera> *persistentCamera = reinterpret_cast<PersistentRef<VRONodeCamera> *>(ptr);
        return persistentCamera->get();
    }
}

class CameraDelegateJNI : public VROCameraDelegate {
public:
    CameraDelegateJNI(jobject obj) {
        _javaObject = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewWeakGlobalRef(obj));
    }

    /*
     Called from VRORenderer to notify the JNI bridge with a camera transformation update.
     Filtering is also performed here to reduce the number of bridge synchronization calls.
     */
    void onCameraTransformationUpdate(VROVector3f pos, VROVector3f rot, VROVector3f forward) {
        if (!shouldUpdate(pos, forward)) {
            return;
        }

        JNIEnv *env = VROPlatformGetJNIEnv();
        jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
        VROPlatformDispatchAsyncApplication([jObjWeak, pos, rot, forward] {
            JNIEnv *env = VROPlatformGetJNIEnv();
            jobject localObj = env->NewLocalRef(jObjWeak);
            if (localObj == NULL) {
                return;
            }
            jfloatArray jPos = ARUtilsCreateFloatArrayFromVector3f(pos);
            jfloatArray jRot = ARUtilsCreateFloatArrayFromVector3f(rot);
            jfloatArray jForward = ARUtilsCreateFloatArrayFromVector3f(forward);
            VROPlatformCallJavaFunction(localObj, "onCameraTransformationUpdate", "([F[F[F)V",
                                        jPos, jRot, jForward);
            env->DeleteLocalRef(localObj);
            env->DeleteWeakGlobalRef(jObjWeak);
        });
    }

    /*
     Returns true if the camera has moved sufficiently beyond a certain distance or rotation
     threshold that warrants a transformation update across the JNI bridge.
     */
    bool shouldUpdate(VROVector3f pos, VROVector3f forward) {
        double currentRenderTime = VROTimeCurrentMillis();
        if (_lastSampleTimeMs + 16 >= currentRenderTime) {
            return false;
        }

        // Determine if we need to flush an update with stale camera data.
        bool shouldBypassFilters = shouldForceStaleUpdate(pos, forward);

        // Only trigger delegates if the camera has moved / rotated a sufficient amount
        // (and if there is no stale transforms to be flushed).
        if (!shouldBypassFilters && _lastPositionUpdate.distance(pos) < _distanceThreshold &&
            _lastForwardVectorUpdate.angleWithVector(forward) < _angleThreshold) {
            return false;
        }

        _lastSampleTimeMs = currentRenderTime;
        _lastForwardVectorUpdate = forward;
        _lastPositionUpdate = pos;
        return true;
    }

    /*
     Returns true to flush a stale camera transform update. This occurs if a new Camera
     transformation has been receieved, but is not significant enough to satisfy the
     distance/rotation thresholds for a stale period length of time.
     */
    bool shouldForceStaleUpdate(VROVector3f pos, VROVector3f forward) {
        // If different, refresh stale counter, return false signaling that we do not need
        // to force a stale update.
        if (_lastSampledPos != pos || _lastSampledForward != forward) {
            _sampledStaleCount = 0;
            _lastSampledPos = pos;
            _lastSampledForward = forward;
            return false;
        }

        // Else If the position and forward remain the same, perform stale checks.
        // Here, we return true if the data is stale with a lifespan of _stalePeriodThreshold.
        if ( _sampledStaleCount < _stalePeriodThreshold) {
            _sampledStaleCount++;
            return false;
        } else if (_sampledStaleCount == _stalePeriodThreshold) {
            _sampledStaleCount++;
        } else if (_sampledStaleCount > _stalePeriodThreshold) {
            return false;
        }

        _lastSampledPos = pos;
        _lastSampledForward = forward;
        return true;
    }

private:
    jobject _javaObject;

    /*
     Last time stamp at which we have been notified of transformation updates.
     */
    double _lastSampleTimeMs = 0;
    VROVector3f _lastSampledPos;
    VROVector3f _lastSampledForward;

    /*
     Distance threshold filters to prevent thrashing the UI thread with updates.
     */
    const double _distanceThreshold = 0.01;
    VROVector3f _lastPositionUpdate;

    /*
     Rotation threshold filters to prevent thrashing the UI thread with updates.
     */
    const double _angleThreshold = 0.017;
    VROVector3f _lastForwardVectorUpdate;

    /*
     Period threshold after which the last known transform is considered stale and thus
     an update flush is required.
     */
    const int _stalePeriodThreshold = 20;
    int _sampledStaleCount = 0;
};

#endif //ANDROID_CAMERA_JNI_H
