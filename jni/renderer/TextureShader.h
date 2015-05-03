#ifndef RENDERER_TEXTURESHADER_H
#define RENDERER_TEXTURESHADER_H

#include "Shader.h"

class TextureShader
:	public Shader
{
protected:
    GLint		m_positionAttributeHandle;
    GLint		m_texCoordAttributeHandle;
    GLint		m_samplerHandle;
    GLint		m_matrixHandle;

public:
    TextureShader();
    virtual ~TextureShader();

    virtual bool link();
    virtual void setup(const GraphicsSprite& sprite, const GLfloat *value) const;
};

#endif // !RENDERER_TEXTURESHADER_H
