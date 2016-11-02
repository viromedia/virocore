//
//  OrientationEKF.h
//  CardboardSDK-iOS
//

#ifndef __CardboardSDK_iOS__OrientationEKF__
#define __CardboardSDK_iOS__OrientationEKF__

#include "VRODefines.h"
#if VRO_METAL

#include "Vector3d.h"
#include "Matrix3x3d.h"

#import <GLKit/GLKit.h>

class OrientationEKF
{
  public:
    OrientationEKF();
    virtual ~OrientationEKF();
    
    void reset();
    bool isReady();
    
    void processGyro(GLKVector3 gyro, double sensorTimeStamp);
    void processAcceleration(GLKVector3 acc, double sensorTimeStamp);
    
    double getHeadingDegrees();
    void setHeadingDegrees(double heading);
    
    GLKMatrix4 getGLMatrix();
    GLKMatrix4 getPredictedGLMatrix(double secondsAfterLastGyroEvent);
    
    
  private:
    Matrix3x3d _so3SensorFromWorld;
    Matrix3x3d _so3LastMotion;
    Matrix3x3d _mP;
    Matrix3x3d _mQ;
    Matrix3x3d _mR;
    Matrix3x3d _mRAcceleration;
    Matrix3x3d _mS;
    Matrix3x3d _mH;
    Matrix3x3d _mK;
    Vector3d _vNu;
    Vector3d _vZ;
    Vector3d _vH;
    Vector3d _vU;
    Vector3d _vX;
    Vector3d _vDown;
    Vector3d _vNorth;
    double _sensorTimeStampGyro;
    GLKVector3 _lastGyro;
    double _previousAccelNorm;
    double _movingAverageAccelNormChange;
    double _filteredGyroTimestep;
    bool _timestepFilterInit;
    int _numGyroTimestepSamples;
    bool _gyroFilterValid;
    bool _alignedToGravity;
    bool _alignedToNorth;
    
    void filterGyroTimestep(double timestep);
    void updateCovariancesAfterMotion();
    void updateAccelerationCovariance(double currentAccelNorm);
    void accelerationObservationFunctionForNumericalJacobian(Matrix3x3d *so3SensorFromWorldPred, Vector3d *result);
};

#endif
#endif
