//
//  VROMagnetSensor.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/26/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef __CardboardSDK_iOS__MagnetSensor__
#define __CardboardSDK_iOS__MagnetSensor__

#import <CoreMotion/CoreMotion.h>
#import <GLKit/GLKit.h>

#include <vector>

class VROMagnetSensor {
    
public:
    
    VROMagnetSensor();
    virtual ~VROMagnetSensor() {}
    void start();
    void stop();
    
private:
    
    CMMotionManager *_manager;
    size_t _sampleIndex;
    GLKVector3 _baseline;
    std::vector<GLKVector3> _sensorData;
    std::vector<float> _offsets;
    
    void addData(GLKVector3 value);
    void evaluateModel();
    void computeOffsets(int start, GLKVector3 baseline);
    
    static const size_t numberOfSamples = 20;
};

#endif