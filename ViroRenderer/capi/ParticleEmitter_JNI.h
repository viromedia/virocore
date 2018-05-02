//
//  ParticleEmitter_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_PARTICLE_EMITTER_JNI_H
#define ANDROID_PARTICLE_EMITTER_JNI_H

#include <memory>
#include <VROPlatformUtil.h>
#include "VROParticleEmitter.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ParticleEmitter {

    inline std::shared_ptr<VROParticleModifier> getParticleModifier(VRO_ENV env,
                                                                    VRO_STRING jFactor,
                                                                    VRO_ARRAY(VRO_FLOAT_ARRAY) jInitialValue,
                                                                    VRO_ARRAY(VRO_FLOAT_ARRAY) jInterpolatedIntervals,
                                                                    VRO_ARRAY(VRO_FLOAT_ARRAY) jInterpolatedValues) {
        // Parse out the initial values for this modifier
        std::vector<VROVector3f> initialValues;
        int numberOfValues = VRO_ARRAY_LENGTH(jInitialValue);
        for (int i = 0; i < numberOfValues; i++) {
            VRO_FLOAT_ARRAY vec3Value = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(jInitialValue, i);
            VRO_FLOAT *vec3ValueArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(vec3Value);
            VROVector3f vec3 = VROVector3f(vec3ValueArray[0], vec3ValueArray[1], vec3ValueArray[2]);
            initialValues.push_back(vec3);
        }

        if (jInitialValue == NULL || jInterpolatedValues == NULL) {
            return std::make_shared<VROParticleModifier>(initialValues[0], initialValues[1]);
        }

        // Grab a reference factor for this modifier against which to interpolate.
        std::string strFactorType = VRO_STRING_STL(jFactor);
        VROParticleModifier::VROModifierFactor bodyType = VROParticleModifier::getModifierFactorForString(strFactorType);

        // Parse out VROModifierIntervals containing sequentially interpolated target values and its
        // corresponding intervals.
        std::vector<VROParticleModifier::VROModifierInterval> interpolatedIntervals;
        int numberOfIntervals = VRO_ARRAY_LENGTH(jInterpolatedIntervals);
        for (int i = 0; i < numberOfIntervals; i++) {
            // Get the interval window this interpolation point applies to
            VRO_FLOAT_ARRAY vec2Value = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(jInterpolatedIntervals, i);
            VRO_FLOAT *jIntervalWindow = VRO_FLOAT_ARRAY_GET_ELEMENTS(vec2Value);

            // Get the value to interpolate towards at the end of this interval window
            VRO_FLOAT_ARRAY vec3Value = (VRO_FLOAT_ARRAY) VRO_ARRAY_GET(jInterpolatedValues, i);
            VRO_FLOAT *jPoint = VRO_FLOAT_ARRAY_GET_ELEMENTS(vec3Value);
            VROVector3f targetedValue = VROVector3f(jPoint[0], jPoint[1], jPoint[2]);

            VROParticleModifier::VROModifierInterval interpolatedPoint;
            interpolatedPoint.startFactor = jIntervalWindow[0];
            interpolatedPoint.endFactor = jIntervalWindow[1];
            interpolatedPoint.targetedValue = targetedValue;

            interpolatedIntervals.push_back(interpolatedPoint);
        }

        return std::make_shared<VROParticleModifier>(initialValues[0],
                                                     initialValues[1],
                                                     bodyType,
                                                     interpolatedIntervals);
    }
}

#endif
