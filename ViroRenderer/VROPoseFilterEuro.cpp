//
//  VROPoseFilterEuro.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/1/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROPoseFilterEuro.h"
#include "VROTime.h"
#include "VROLog.h"
#include <cmath>
#include <ctime>

// To tune these values:
// To minimize jitter and lag when tracking human motion, the two parameters (fcmin and beta)
// can be set using a simple two-step procedure. First beta is set to 0 and fcmin (mincutoff)
// to a reasonable middle-ground value such as 1 Hz. Then the body part is held steady or moved
// at a very low speed while fcmin is adjusted to remove jitter and preserve an acceptable lag
// during these slow movements (decreasing fcmin reduces jitter but increases lag, fcmin must be > 0).
// Next, the body part is moved quickly in different directions while beta is increased with a
// focus on minimizing lag. First find the right order of magnitude to tune beta, which depends
// on the kind of data you manipulate and their units: do not hesitate to start with values like
// 0.001 or 0.0001. You can first multiply and divide beta by factor 10 until you notice an effect
// on latency when moving quickly.
//
// Note that parameters fcmin and beta have clear conceptual relationships: if high speed lag is
// a problem, increase beta; if slow speed jitter is a problem, decrease fcmin.
static const double kEuroBeta = 1.0;
static const double kEuroFCMin = 1.7;

// -----------------------------------------------------------------

static const double kFilterUndefinedTime = -1.0;

LowPassFilter::LowPassFilter(double alpha, VROVector3f initialValue) {
    _lastRaw = _lastFiltered = initialValue;
    _alpha = alpha;
    _initialized = false;
}

VROVector3f LowPassFilter::filter(VROVector3f value, double alpha) {
    VROVector3f result ;
    if (_initialized)
        result = alpha * value + (1.0 - alpha) * _lastFiltered;
    else {
        result = value;
        _initialized = true;
    }
    _lastRaw = value;
    _lastFiltered = result;
    return result;
}

// -----------------------------------------------------------------

OneEuroFilter::OneEuroFilter(double initialFrequency, double minCutoff, double beta, double derivativeCutoff) {
    _frequency = initialFrequency;
    _minFrequencyCutoff = minCutoff;
    _beta = beta;
    _derivativeCutoff = derivativeCutoff;
    
    _x = new LowPassFilter(computeAlpha(minCutoff));
    _dx = new LowPassFilter(computeAlpha(derivativeCutoff));
    _lastTimestamp = kFilterUndefinedTime;
}

OneEuroFilter::~OneEuroFilter() {
    delete(_x);
    delete(_dx);
}

VROVector3f OneEuroFilter::filter(VROVector3f value, double timestamp, bool debug) {
    // Update the sampling frequency based on timestamps
    if (_lastTimestamp != kFilterUndefinedTime && timestamp != kFilterUndefinedTime) {
        _frequency = 1.0 / (timestamp - _lastTimestamp);
    }
    _lastTimestamp = timestamp;
    if (isinf(_frequency)) {
        return value;
    }
    
    // Estimate the current variation per second
    VROVector3f dvalue = _x->hasLastRawValue() ? (value - _x->getLastRawValue()) * _frequency : VROVector3f(0, 0, 0);
    VROVector3f edvalue = _dx->filter(dvalue, computeAlpha(_derivativeCutoff));
    
    // Update the cutoff frequency: this should increase as edvalue increases
    double cutoff = _minFrequencyCutoff + _beta * edvalue.magnitude();
    if (debug) {
        pinfo("Cutoff is now %f derived from magnitude %f", cutoff, edvalue.magnitude());
    }
    
    // Filter with the new alpha derived from the cutoff
    return _x->filter(value, computeAlpha(cutoff));
}

double OneEuroFilter::computeAlpha(double cutoff) {
    double te = 1.0 / _frequency ;
    double tau = 1.0 / (2 * M_PI * cutoff);
    return 1.0 / (1.0 + tau / te);
}

// -----------------------------------------------------------------

VROPoseFilterEuro::VROPoseFilterEuro(float trackingPeriodMs, float confidenceThreshold) :
    VROPoseFilter(trackingPeriodMs, confidenceThreshold) {
    
    double frequency = 60;
    double dcutoff = 1.0;
    for (int i = 0; i < kNumBodyJoints; i++) {
        _filters.push_back(std::make_shared<OneEuroFilter>(frequency, kEuroFCMin, kEuroBeta, dcutoff));
    }
}

VROPoseFilterEuro::~VROPoseFilterEuro() {

}

VROPoseFrame VROPoseFilterEuro::temporalFilter(const std::vector<VROPoseFrame> &frames, const VROPoseFrame &combinedFrame,
                                               const VROPoseFrame &newFrame) {
    VROPoseFrame dampenedJoints = newPoseFrame();
    for (int i = 0; i < kNumBodyJoints; i++) {
        VROBodyJointType type = (VROBodyJointType) i;
        
        // Compute filtered position
        const std::vector<VROInferredBodyJoint> &samples = newFrame[i];
        if (samples.empty()) {
            continue;
        }
        const VROInferredBodyJoint &sample = samples.front();
        
        VROVector3f filtered = _filters[i]->filter(sample.getCenter(), sample.getCreationTime() / 1000.0,
                                                   false);
        
        // Compute aggregate confidence
        const std::vector<VROInferredBodyJoint> &confidenceSamples = combinedFrame[i];
        if (confidenceSamples.empty()) {
            continue;
        }
        float sumConfidence = 0;
        for (int i = 0; i < confidenceSamples.size(); i++) {
            sumConfidence += confidenceSamples[i].getConfidence();
        }
        
        VROInferredBodyJoint dampenedJoint(type);
        dampenedJoint.setCenter(filtered);
        dampenedJoint.setConfidence(sumConfidence / (float) confidenceSamples.size());
        dampenedJoints[i] = { dampenedJoint };
    }
    
    return dampenedJoints;
}
