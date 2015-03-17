#ifndef RENDERER_BASICSHADER_H
#define RENDERER_BASICSHADER_H

#include "Shader.h"

class BasicShader
:	public Shader
{
public:
    GLint	m_positionAttributeHandle;

public:
    BasicShader();
    virtual ~BasicShader();

    virtual void Link();
    virtual void Setup(GraphicsSprite& sprite);
};

#endif // !RENDERER_BASICSHADER_H
