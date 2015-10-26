//
//  OrientationEKF.mm
//  CardboardSDK-iOS
//

#include "OrientationEKF.h"

#include "SO3Util.h"

#include <cmath>
#include <algorithm>

static const double DEG_TO_RAD = M_PI / 180.0;
static const double RAD_TO_DEG = 180.0 / M_PI;

GLKMatrix4 glMatrixFromSo3(Matrix3x3d *so3)
{
    GLKMatrix4 rotationMatrix;
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            rotationMatrix.m[(4 * c + r)] = so3->get(r, c);
        }
    }
    rotationMatrix.m[3] = 0.0;
    rotationMatrix.m[7] = 0.0;
    rotationMatrix.m[11] = 0.0;
    rotationMatrix.m[12] = 0.0;
    rotationMatrix.m[13] = 0.0;
    rotationMatrix.m[14] = 0.0;
    rotationMatrix.m[15] = 1.0;
    return rotationMatrix;
}

OrientationEKF::OrientationEKF() :
    _previousAccelNorm(0.0),
    _movingAverageAccelNormChange(0.0),
    _timestepFilterInit(false),
    _gyroFilterValid(true)
    
{
    reset();
}

OrientationEKF::~OrientationEKF()
{
}

void OrientationEKF::reset()
{
    _sensorTimeStampGyro = 0.0;
    _so3SensorFromWorld.setIdentity();
    _so3LastMotion.setIdentity();
    _mP.setZero();
    _mP.setSameDiagonal(25.0);
    _mQ.setZero();
    _mQ.setSameDiagonal(1.0);
    _mR.setZero();
    _mR.setSameDiagonal(0.0625);
    _mRAcceleration.setZero();
    _mRAcceleration.setSameDiagonal(0.5625);
    _mS.setZero();
    _mH.setZero();
    _mK.setZero();
    _vNu.setZero();
    _vZ.setZero();
    _vH.setZero();
    _vU.setZero();
    _vX.setZero();
    // Flipped from Android so it uses the same convention as CoreMotion
    // was: _vDown.set(0.0, 0.0, 9.81);
    _vDown.set(0.0, 0.0, -9.81);
    _vNorth.set(0.0, 1.0, 0.0);
    _alignedToGravity = false;
    _alignedToNorth = false;
}

bool OrientationEKF::isReady()
{
    return _alignedToGravity;
}

double OrientationEKF::getHeadingDegrees()
{
    double x = _so3SensorFromWorld.get(2, 0);
    double y = _so3SensorFromWorld.get(2, 1);
    double mag = sqrt(x * x + y * y);
    if (mag < 0.1) {
        return 0.0;
    }
    double heading = -90.0 - atan2(y, x) * RAD_TO_DEG;
    if (heading < 0.0) {
        heading += 360.0;
    }
    if (heading >= 360.0) {
        heading -= 360.0;
    }
    return heading;
}

void OrientationEKF::setHeadingDegrees(double heading)
{
    double currentHeading = getHeadingDegrees();
    double deltaHeading = heading - currentHeading;
    double s = sin(deltaHeading * DEG_TO_RAD);
    double c = cos(deltaHeading * DEG_TO_RAD);
    Matrix3x3d deltaHeadingRotationMatrix(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);
    Matrix3x3d::mult(&_so3SensorFromWorld, &deltaHeadingRotationMatrix, &_so3SensorFromWorld);
}

GLKMatrix4 OrientationEKF::getGLMatrix()
{
    return glMatrixFromSo3(&_so3SensorFromWorld);
}

GLKMatrix4 OrientationEKF::getPredictedGLMatrix(double secondsAfterLastGyroEvent)
{
    double dT = secondsAfterLastGyroEvent;
    Vector3d pmu(_lastGyro.x * -dT, _lastGyro.y * -dT, _lastGyro.z * -dT);
    Matrix3x3d so3PredictedMotion;
    SO3Util::so3FromMu(&pmu, &so3PredictedMotion);
    Matrix3x3d so3PredictedState;
    Matrix3x3d::mult(&so3PredictedMotion, &_so3SensorFromWorld, &so3PredictedState);
    return glMatrixFromSo3(&so3PredictedState);
}

void OrientationEKF::processGyro(GLKVector3 gyro, double sensorTimeStamp)
{
    if (_sensorTimeStampGyro != 0.0) {
        
        double dT = sensorTimeStamp - _sensorTimeStampGyro;
        if (dT > 0.04f) {
            dT = _gyroFilterValid ? _filteredGyroTimestep : 0.01;
        } else {
            filterGyroTimestep(dT);
        }
        
        _vU.set(gyro.x * -dT, gyro.y * -dT, gyro.z * -dT);
        SO3Util::so3FromMu(&_vU, &_so3LastMotion);
        Matrix3x3d::mult(&_so3LastMotion, &_so3SensorFromWorld, &_so3SensorFromWorld);
        updateCovariancesAfterMotion();
        Matrix3x3d temp;
        temp.set(&_mQ);
        temp.scale(dT * dT);
        _mP.plusEquals(&temp);
        
    }
    _sensorTimeStampGyro = sensorTimeStamp;
    _lastGyro = gyro;
}

void OrientationEKF::processAcceleration(GLKVector3 acc, double sensorTimeStamp)
{
    _vZ.set(acc.x, acc.y, acc.z);
    updateAccelerationCovariance(_vZ.length());
    if (_alignedToGravity)
    {
        accelerationObservationFunctionForNumericalJacobian(&_so3SensorFromWorld, &_vNu);
        const double eps = 1.0E-7;
        for (int dof = 0; dof < 3; dof++)
        {
            Vector3d delta;
            delta.setZero();
            delta.setComponent(dof, eps);
            Matrix3x3d tempM;
            SO3Util::so3FromMu(&delta, &tempM);
            Matrix3x3d::mult(&tempM, &_so3SensorFromWorld, &tempM);
            Vector3d tempV;
            accelerationObservationFunctionForNumericalJacobian(&tempM, &tempV);
            Vector3d::sub(&_vNu, &tempV, &tempV);
            tempV.scale(1.0/eps);
            _mH.setColumn(dof, &tempV);
        }
        
        
        Matrix3x3d mHt;
        _mH.transpose(&mHt);
        Matrix3x3d temp;
        Matrix3x3d::mult(&_mP, &mHt, &temp);
        Matrix3x3d::mult(&_mH, &temp, &temp);
        Matrix3x3d::add(&temp, &_mRAcceleration, &_mS);
        _mS.invert(&temp);
        Matrix3x3d::mult(&mHt, &temp, &temp);
        Matrix3x3d::mult(&_mP, &temp, &_mK);
        Matrix3x3d::mult(&_mK, &_vNu, &_vX);
        Matrix3x3d::mult(&_mK, &_mH, &temp);
        Matrix3x3d temp2;
        temp2.setIdentity();
        temp2.minusEquals(&temp);
        Matrix3x3d::mult(&temp2, &_mP, &_mP);
        SO3Util::so3FromMu(&_vX, &_so3LastMotion);
        Matrix3x3d::mult(&_so3LastMotion, &_so3SensorFromWorld, &_so3SensorFromWorld);
        updateCovariancesAfterMotion();
    }
    else
    {
        SO3Util::so3FromTwoVecN(&_vDown, &_vZ, &_so3SensorFromWorld);
        _alignedToGravity = true;
    }
}

void OrientationEKF::filterGyroTimestep(double timestep)
{
    const double kFilterCoeff = 0.95;
    if (!_timestepFilterInit)
    {
        _filteredGyroTimestep = timestep;
        _numGyroTimestepSamples = 1;
        _timestepFilterInit = true;
    }
    else
    {
        _filteredGyroTimestep = kFilterCoeff * _filteredGyroTimestep + (1.0-kFilterCoeff) * timestep;
        ++_numGyroTimestepSamples;
        _gyroFilterValid = (_numGyroTimestepSamples > 10);
    }
}

void OrientationEKF::updateCovariancesAfterMotion()
{
    Matrix3x3d temp;
    _so3LastMotion.transpose(&temp);
    Matrix3x3d::mult(&_mP, &temp, &temp);
    Matrix3x3d::mult(&_so3LastMotion, &temp, &_mP);
    _so3LastMotion.setIdentity();
}

void OrientationEKF::updateAccelerationCovariance(double currentAccelNorm)
{
    double currentAccelNormChange = fabs(currentAccelNorm - _previousAccelNorm);
    _previousAccelNorm = currentAccelNorm;
    const double kSmoothingFactor = 0.5;
    _movingAverageAccelNormChange = kSmoothingFactor * _movingAverageAccelNormChange + (1.0-kSmoothingFactor) * currentAccelNormChange;
    const double kMaxAccelNormChange = 0.15;
    const double kMinAccelNoiseSigma = 0.75;
    const double kMaxAccelNoiseSigma = 7.0;
    double normChangeRatio = _movingAverageAccelNormChange / kMaxAccelNormChange;
    double accelNoiseSigma = std::min(kMaxAccelNoiseSigma, kMinAccelNoiseSigma + normChangeRatio * (kMaxAccelNoiseSigma-kMinAccelNoiseSigma));
    _mRAcceleration.setSameDiagonal(accelNoiseSigma * accelNoiseSigma);
}

void OrientationEKF::accelerationObservationFunctionForNumericalJacobian(Matrix3x3d *so3SensorFromWorldPred, Vector3d *result)
{
    Matrix3x3d::mult(so3SensorFromWorldPred, &_vDown, &_vH);
    Matrix3x3d temp;
    SO3Util::so3FromTwoVecN(&_vH, &_vZ, &temp);
    SO3Util::muFromSO3(&temp, result);
}
