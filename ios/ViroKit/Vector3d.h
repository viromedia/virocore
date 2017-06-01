//
//  Vector3d.h
//  CardboardSDK-iOS
//

#ifndef __CardboardSDK_iOS__Vector3d__
#define __CardboardSDK_iOS__Vector3d__

#include "VRODefines.h"

class Vector3d
{
    friend class Matrix3x3d;
    friend class SO3Util;

  public:
    Vector3d();
    Vector3d(double x, double y, double z);
    Vector3d(Vector3d *other);
    void set(double x, double y, double z);
    void setComponent(int i, double val);
    void setZero();
    void set(Vector3d *other);
    void scale(double s);
    void normalize();
    static double dot(Vector3d *a, Vector3d *b);
    double length();
    bool sameValues(Vector3d *other);
    static void sub(Vector3d *a, Vector3d *b, Vector3d *result);
    static void cross(Vector3d *a, Vector3d *b, Vector3d *result);
    static void ortho(Vector3d *v, Vector3d *result);
    static int largestAbsComponent(Vector3d *v);
    
    inline double x() const { return _x; }
    inline double y() const { return _y; }
    inline double z() const { return _z; }

  private:
    double _x;
    double _y;
    double _z;
};

#endif
