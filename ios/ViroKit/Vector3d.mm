//
//  Vector3d.mm
//  CardboardSDK-iOS
//

#include "Vector3d.h"
#include <cmath>

Vector3d::Vector3d()
{
    setZero();
}

Vector3d::Vector3d(Vector3d *v)
{
    set(v->_x, v->_y, v->_z);
}

Vector3d::Vector3d(double x, double y, double z)
{
    set(x, y, z);
}

void Vector3d::set(double x, double y, double z)
{
    _x = x;
    _y = y;
    _z = z;
}

void Vector3d::setComponent(int i, double val)
{
    if (i == 0)
    {
        _x = val;
    }
    else if (i == 1)
    {
        _y = val;
    }
    else
    {
        _z = val;
    }
}

void Vector3d::setZero()
{
    set(0, 0, 0);
}

void Vector3d::set(Vector3d *other)
{
    set(other->_x, other->_y, other->_z);
}

void Vector3d::scale(double s)
{
    _x *= s;
    _y *= s;
    _z *= s;
}

void Vector3d::normalize()
{
    double d = length();
    if (d != 0.0)
    {
        scale(1.0 / d);
    }
}

double Vector3d::dot(Vector3d *a, Vector3d *b)
{
    return a->_x * b->_x + a->_y * b->_y + a->_z * b->_z;
}

double Vector3d::length()
{
    return sqrt(_x * _x + _y * _y + _z * _z);
}

bool Vector3d::sameValues(Vector3d *other)
{
    return (_x == other->_x) && (_y == other->_y) && (_z == other->_z);
}

void Vector3d::sub(Vector3d *a, Vector3d *b, Vector3d *result)
{
    result->set(a->_x - b->_x,
                a->_y - b->_y,
                a->_z - b->_z);
}

void Vector3d::cross(Vector3d *a, Vector3d *b, Vector3d *result)
{
    result->set(a->_y * b->_z - a->_z * b->_y,
                a->_z * b->_x - a->_x * b->_z,
                a->_x * b->_y - a->_y * b->_x);
}

void Vector3d::ortho(Vector3d *v, Vector3d *result)
{
    int k = largestAbsComponent(v) - 1;
    if (k < 0)
    {
        k = 2;
    }
    result->setZero();
    result->setComponent(k, 1.0);
    cross(v, result, result);
    result->normalize();
}

int Vector3d::largestAbsComponent(Vector3d *v)
{
    double xAbs = fabs(v->_x);
    double yAbs = fabs(v->_y);
    double zAbs = fabs(v->_z);
    if (xAbs > yAbs)
    {
        if (xAbs > zAbs)
        {
            return 0;
        }
        return 2;
    }
    if (yAbs > zAbs)
    {
        return 1;
    }
    return 2;
}
