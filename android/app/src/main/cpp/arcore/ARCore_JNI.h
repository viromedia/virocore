//
//  Frame_JNI.h
//  Viro
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARCORE_JNI_h
#define ARCORE_JNI_h

#include <jni/jni.hpp>
#include <jni/class.hpp>
#include <jni/object.hpp>
#include "VROViewport.h"

class VROMatrix4f;

namespace arcore {

    struct Config;
    struct LightEstimate;
    struct Frame;
    struct Session;

    namespace config {

        enum class LightingMode {
            Disabled,
            AmbientIntensity
        };
        enum class PlaneFindingMode {
            Disabled,
            Horizontal
        };
        enum class UpdateMode {
            Blocking,
            LatestCameraImage
        };

        jni::Object<Config> getConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                      UpdateMode updateMode);

    }

    namespace light_estimate {

        jni::jfloat getPixelIntensity(jni::Object<LightEstimate> lightEstimate);
        jni::jboolean isValid(jni::Object<LightEstimate> lightEstimate);

    }

    namespace frame {

        enum class TrackingState {
            NotTracking,
            Tracking
        };

        VROMatrix4f getViewMatrix(jni::Object<Frame> frame);
        TrackingState getTrackingState(jni::Object<Frame> frame);
        jni::Object<LightEstimate> getLightEstimate(jni::Object<Frame> frame);
        bool isDisplayRotationChanged(jni::Object<Frame> frame);
        jni::jlong getTimestampNs(jni::Object<Frame> frame);

    }

    namespace session {

        VROMatrix4f getProjectionMatrix(jni::Object<Session> session, float near, float far);
        void setCameraTextureName(jni::Object<Session> session, jni::jint textureName);
        void pause(jni::Object<Session> session);
        jni::Object<Frame> update(jni::Object<Session> session);

    }
}

#endif /* ARCORE_JNI_h */
