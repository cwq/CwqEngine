/**
 Original file from GamePlay3D: http://gameplay3d.org
 */
#include "Quaternion.h"

NS_CWQ_MATH_BEGIN

inline const Quaternion Quaternion::operator*(const Quaternion& q) const
{
    Quaternion result(*this);
    result.multiply(q);
    return result;
}

inline Quaternion& Quaternion::operator*=(const Quaternion& q)
{
    multiply(q);
    return *this;
}

NS_CWQ_MATH_END
