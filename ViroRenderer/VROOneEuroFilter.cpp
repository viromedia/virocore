//
//  VROOneEuroFilter.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROOneEuroFilter.h"
#include "VROLog.h"

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

VROOneEuroFilter::VROOneEuroFilter(double initialFrequency, double minCutoff, double beta, double derivativeCutoff) {
    _frequency = initialFrequency;
    _minFrequencyCutoff = minCutoff;
    _beta = beta;
    _derivativeCutoff = derivativeCutoff;
    
    _x = new LowPassFilter(computeAlpha(minCutoff));
    _dx = new LowPassFilter(computeAlpha(derivativeCutoff));
    _lastTimestamp = kFilterUndefinedTime;
}

VROOneEuroFilter::~VROOneEuroFilter() {
    delete(_x);
    delete(_dx);
}

VROVector3f VROOneEuroFilter::filter(VROVector3f value, double timestamp, bool debug) {
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

double VROOneEuroFilter::computeAlpha(double cutoff) {
    double te = 1.0 / _frequency ;
    double tau = 1.0 / (2 * M_PI * cutoff);
    return 1.0 / (1.0 + tau / te);
}

// -----------------------------------------------------------------

LowPassFilterF::LowPassFilterF(double alpha, float initialValue) {
    _lastRaw = _lastFiltered = initialValue;
    _alpha = alpha;
    _initialized = false;
}

float LowPassFilterF::filter(float value, double alpha) {
    float result;
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

VROOneEuroFilterF::VROOneEuroFilterF(double initialFrequency, double minCutoff, double beta, double derivativeCutoff) {
    _frequency = initialFrequency;
    _minFrequencyCutoff = minCutoff;
    _beta = beta;
    _derivativeCutoff = derivativeCutoff;
    
    _x = new LowPassFilterF(computeAlpha(minCutoff));
    _dx = new LowPassFilterF(computeAlpha(derivativeCutoff));
    _lastTimestamp = kFilterUndefinedTime;
}

VROOneEuroFilterF::~VROOneEuroFilterF() {
    delete(_x);
    delete(_dx);
}

float VROOneEuroFilterF::filter(float value, double timestamp, bool debug) {
    // Update the sampling frequency based on timestamps
    if (_lastTimestamp != kFilterUndefinedTime && timestamp != kFilterUndefinedTime) {
        _frequency = 1.0 / (timestamp - _lastTimestamp);
    }
    _lastTimestamp = timestamp;
    if (isinf(_frequency)) {
        return value;
    }
    
    // Estimate the current variation per second
    float dvalue = _x->hasLastRawValue() ? (value - _x->getLastRawValue()) * _frequency : 0;
    float edvalue = _dx->filter(dvalue, computeAlpha(_derivativeCutoff));
    
    // Update the cutoff frequency: this should increase as edvalue increases
    double cutoff = _minFrequencyCutoff + _beta * fabs(edvalue);
    if (debug) {
        pinfo("Cutoff is now %f derived from magnitude %f", cutoff, fabs(edvalue));
    }
    
    // Filter with the new alpha derived from the cutoff
    return _x->filter(value, computeAlpha(cutoff));
}

double VROOneEuroFilterF::computeAlpha(double cutoff) {
    double te = 1.0 / _frequency ;
    double tau = 1.0 / (2 * M_PI * cutoff);
    return 1.0 / (1.0 + tau / te);
}
