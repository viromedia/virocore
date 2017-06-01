//
//  Matrix3x3d.mm
//  CardboardSDK-iOS
//

#include "Matrix3x3d.h"

Matrix3x3d::Matrix3x3d()
{
    setZero();
}

Matrix3x3d::Matrix3x3d(double m00, double m01, double m02,
                       double m10, double m11, double m12,
                       double m20, double m21, double m22)
{
    _m[0] = m00;
    _m[1] = m01;
    _m[2] = m02;
    _m[3] = m10;
    _m[4] = m11;
    _m[5] = m12;
    _m[6] = m20;
    _m[7] = m21;
    _m[8] = m22;
}

Matrix3x3d::Matrix3x3d(Matrix3x3d *o)
{
    for (int i = 0; i < 9; i++)
    {
        _m[i] = o->_m[i];
    }
}

void Matrix3x3d::set(double m00, double m01, double m02,
                     double m10, double m11, double m12,
                     double m20, double m21, double m22)
{
    _m[0] = m00;
    _m[1] = m01;
    _m[2] = m02;
    _m[3] = m10;
    _m[4] = m11;
    _m[5] = m12;
    _m[6] = m20;
    _m[7] = m21;
    _m[8] = m22;
}

void Matrix3x3d::set(Matrix3x3d *o)
{
    for (int i = 0; i < 9; i++)
    {
        _m[i] = o->_m[i];
    }
}

void Matrix3x3d::setZero()
{
    for (int i = 0; i < 9; i++)
    {
        _m[i] = 0;
    }
}

void Matrix3x3d::setIdentity()
{
    _m[0] = 1;
    _m[1] = 0;
    _m[2] = 0;
    _m[3] = 0;
    _m[4] = 1;
    _m[5] = 0;
    _m[6] = 0;
    _m[7] = 0;
    _m[8] = 1;
}

void Matrix3x3d::setSameDiagonal(double d)
{
    _m[0] = d;
    _m[4] = d;
    _m[8] = d;
}

double Matrix3x3d::get(int row, int col)
{
    return _m[(3 * row + col)];
}

void Matrix3x3d::set(int row, int col, double value)
{
    _m[(3 * row + col)] = value;
}

void Matrix3x3d::getColumn(int col, Vector3d *v)
{
    v->_x = _m[col];
    v->_y = _m[col + 3];
    v->_z = _m[col + 6];
}

void Matrix3x3d::setColumn(int col, Vector3d *v)
{
    _m[col] = v->_x;
    _m[col + 3] = v->_y;
    _m[col + 6] = v->_z;
}

void Matrix3x3d::scale(double s)
{
    for (int i = 0; i < 9; i++)
    {
        _m[i] *= s;
    }
}

void Matrix3x3d::plusEquals(Matrix3x3d *b)
{
    for (int i = 0; i < 9; i++)
    {
        _m[i] += b->_m[i];
    }
}

void Matrix3x3d::minusEquals(Matrix3x3d *b)
{
    for (int i = 0; i < 9; i++)
    {
        _m[i] -= b->_m[i];
    }
}

void Matrix3x3d::transpose()
{
    double tmp = _m[1];
    _m[1] = _m[3];
    _m[3] = tmp;
    tmp = _m[2];
    _m[2] = _m[6];
    _m[6] = tmp;
    tmp = _m[5];
    _m[5] = _m[7];
    _m[7] = tmp;
}

void Matrix3x3d::transpose(Matrix3x3d *result)
{
    result->_m[0] = _m[0];
    result->_m[1] = _m[3];
    result->_m[2] = _m[6];
    result->_m[3] = _m[1];
    result->_m[4] = _m[4];
    result->_m[5] = _m[7];
    result->_m[6] = _m[2];
    result->_m[7] = _m[5];
    result->_m[8] = _m[8];
}

void Matrix3x3d::add(Matrix3x3d *a, Matrix3x3d *b, Matrix3x3d *result)
{
    for (int i = 0; i < 9; i++)
    {
        result->_m[i] = a->_m[i] + b->_m[i];
    }
}

void Matrix3x3d::mult(Matrix3x3d *a, Matrix3x3d *b, Matrix3x3d *result)
{
    result->set(a->_m[0] * b->_m[0] + a->_m[1] * b->_m[3] + a->_m[2] * b->_m[6],
                a->_m[0] * b->_m[1] + a->_m[1] * b->_m[4] + a->_m[2] * b->_m[7],
                a->_m[0] * b->_m[2] + a->_m[1] * b->_m[5] + a->_m[2] * b->_m[8],
                a->_m[3] * b->_m[0] + a->_m[4] * b->_m[3] + a->_m[5] * b->_m[6],
                a->_m[3] * b->_m[1] + a->_m[4] * b->_m[4] + a->_m[5] * b->_m[7],
                a->_m[3] * b->_m[2] + a->_m[4] * b->_m[5] + a->_m[5] * b->_m[8],
                a->_m[6] * b->_m[0] + a->_m[7] * b->_m[3] + a->_m[8] * b->_m[6],
                a->_m[6] * b->_m[1] + a->_m[7] * b->_m[4] + a->_m[8] * b->_m[7],
                a->_m[6] * b->_m[2] + a->_m[7] * b->_m[5] + a->_m[8] * b->_m[8]);
}

void Matrix3x3d::mult(Matrix3x3d *a, Vector3d *v, Vector3d *result)
{
    result->set(a->_m[0] * v->_x + a->_m[1] * v->_y + a->_m[2] * v->_z,
                a->_m[3] * v->_x + a->_m[4] * v->_y + a->_m[5] * v->_z,
                a->_m[6] * v->_x + a->_m[7] * v->_y + a->_m[8] * v->_z);
}

double Matrix3x3d::determinant()
{
    return get(0, 0) * (get(1, 1) * get(2, 2) - get(2, 1) * get(1, 2))
         - get(0, 1) * (get(1, 0) * get(2, 2) - get(1, 2) * get(2, 0))
         + get(0, 2) * (get(1, 0) * get(2, 1) - get(1, 1) * get(2, 0));
}

bool Matrix3x3d::invert(Matrix3x3d *result)
{
    double d = determinant();
    if (d == 0.0)
    {
        return false;
    }
    double invdet = 1.0 / d;
    result->set( (_m[4] * _m[8] - _m[7] * _m[5]) * invdet,
                -(_m[1] * _m[8] - _m[2] * _m[7]) * invdet,
                 (_m[1] * _m[5] - _m[2] * _m[4]) * invdet,
                -(_m[3] * _m[8] - _m[5] * _m[6]) * invdet,
                 (_m[0] * _m[8] - _m[2] * _m[6]) * invdet,
                -(_m[0] * _m[5] - _m[3] * _m[2]) * invdet,
                 (_m[3] * _m[7] - _m[6] * _m[4]) * invdet,
                -(_m[0] * _m[7] - _m[6] * _m[1]) * invdet,
                 (_m[0] * _m[4] - _m[3] * _m[1]) * invdet);
    return true;
}

GLKMatrix3 Matrix3x3d::getGLKMatrix()
{
    return GLKMatrix3Make(_m[0], _m[1], _m[2],
                          _m[3], _m[4], _m[5],
                          _m[6], _m[7], _m[8]);
}
