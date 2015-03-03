/**
 Original file from GamePlay3D: http://gameplay3d.org
 */

#include "math/Vec2.h"

NS_CWQ_MATH_BEGIN

inline const Vec2 Vec2::operator+(const Vec2& v) const
{
    Vec2 result(*this);
    result.add(v);
    return result;
}

inline Vec2& Vec2::operator+=(const Vec2& v)
{
    add(v);
    return *this;
}

inline const Vec2 Vec2::operator-(const Vec2& v) const
{
    Vec2 result(*this);
    result.subtract(v);
    return result;
}

inline Vec2& Vec2::operator-=(const Vec2& v)
{
    subtract(v);
    return *this;
}

inline const Vec2 Vec2::operator-() const
{
    Vec2 result(*this);
    result.negate();
    return result;
}

inline const Vec2 Vec2::operator*(float s) const
{
    Vec2 result(*this);
    result.scale(s);
    return result;
}

inline Vec2& Vec2::operator*=(float s)
{
    scale(s);
    return *this;
}

inline const Vec2 Vec2::operator/(const float s) const
{
    return Vec2(this->x / s, this->y / s);
}

inline bool Vec2::operator<(const Vec2& v) const
{
    if (x == v.x)
    {
        return y < v.y;
    }
    return x < v.x;
}

inline bool Vec2::operator==(const Vec2& v) const
{
    return x==v.x && y==v.y;
}

inline bool Vec2::operator!=(const Vec2& v) const
{
    return x!=v.x || y!=v.y;
}

inline const Vec2 operator*(float x, const Vec2& v)
{
    Vec2 result(v);
    result.scale(x);
    return result;
}

NS_CWQ_MATH_END
