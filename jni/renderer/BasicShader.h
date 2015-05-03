#ifndef RENDERER_BASICSHADER_H
#define RENDERER_BASICSHADER_H

#include "Shader.h"

class BasicShader
:	public Shader
{
protected:
    GLint	m_positionAttributeHandle;

public:
    BasicShader();
    virtual ~BasicShader();

    virtual bool link();
    virtual void setup(const GraphicsSprite& sprite, const GLfloat *mvpMatrix) const;
};

#endif // !RENDERER_BASICSHADER_H
