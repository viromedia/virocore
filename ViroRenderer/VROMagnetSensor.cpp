//
//  VROMagnetSensor.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/26/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMagnetSensor.h"
#include <algorithm>

VROMagnetSensor::VROMagnetSensor() :
    _sampleIndex(0),
    _sensorData(2 * numberOfSamples),
    _offsets(numberOfSamples)
{
    _manager = [[CMMotionManager alloc] init];
}

void VROMagnetSensor::start()
{
    if (_manager.isMagnetometerAvailable && !_manager.isMagnetometerActive)
    {
        _manager.magnetometerUpdateInterval = 1.0f / 100.0f;
        NSOperationQueue *magnetometerQueue = [[NSOperationQueue alloc] init];
        [_manager startMagnetometerUpdatesToQueue:magnetometerQueue
                                      withHandler:^(CMMagnetometerData *magnetometerData, NSError *error)
        {
            addData(GLKVector3 {
                (float) magnetometerData.magneticField.x,
                (float) magnetometerData.magneticField.y,
                (float) magnetometerData.magneticField.z});
        }];
    }
}

void VROMagnetSensor::stop()
{
    [_manager stopMagnetometerUpdates];
}

void VROMagnetSensor::addData(GLKVector3 value)
{
    _sensorData[_sampleIndex % (2 * numberOfSamples)] = value;
    _baseline = value;
    ++_sampleIndex;
    evaluateModel();
}

void VROMagnetSensor::evaluateModel()
{
    if (_sampleIndex < (2 * numberOfSamples))
    {
        return;
    }
    float minimums[2];
    float maximums[2];
    for (int i = 0; i < 2; i++)
    {
        computeOffsets(i * numberOfSamples, _baseline);
        minimums[i] = *std::min_element(_offsets.begin(), _offsets.end());
        maximums[i] = *std::max_element(_offsets.begin(), _offsets.end());
    }
        
    if (minimums[0] < 30.0f && maximums[1] > 130.0f)
    {
        _sampleIndex = 0;
//        [[NSNotificationCenter defaultCenter] postNotificationName:VROTriggerPressedNotification object:nil];
    }
}

void VROMagnetSensor::computeOffsets(int start, GLKVector3 baseline)
{
    size_t frontIndex = _sampleIndex % (2 * numberOfSamples); // currently the oldest sample
    for (int i = 0; i < numberOfSamples; i++)
    {
        GLKVector3 point = _sensorData[(frontIndex + start + i) % (2 * numberOfSamples)];
        float o[] = {point.x - baseline.x, point.y - baseline.y, point.z - baseline.z};
        float magnitude = (float)sqrt(o[0] * o[0] + o[1] * o[1] + o[2] * o[2]);
        _offsets[i] = magnitude;
    }
}
