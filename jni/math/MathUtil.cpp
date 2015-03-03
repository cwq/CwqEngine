/**
 Original file from GamePlay3D: http://gameplay3d.org
 */

#include "MathUtil.h"
#include "base/CWQMacros.h"

NS_CWQ_MATH_BEGIN

void MathUtil::smooth(float* x, float target, float elapsedTime, float responseTime)
{
    GP_ASSERT(x);

    if (elapsedTime > 0)
    {
        *x += (target - *x) * elapsedTime / (elapsedTime + responseTime);
    }
}

void MathUtil::smooth(float* x, float target, float elapsedTime, float riseTime, float fallTime)
{
    GP_ASSERT(x);
    
    if (elapsedTime > 0)
    {
        float delta = target - *x;
        *x += delta * elapsedTime / (elapsedTime + (delta > 0 ? riseTime : fallTime));
    }
}

NS_CWQ_MATH_END
