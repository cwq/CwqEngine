#ifndef RENDERER_TEXTURESHADER_H
#define RENDERER_TEXTURESHADER_H

#include "Shader.h"

//class Texture2D;

class TextureShader
:	public Shader
{
public:
//    Texture2D*	m_pTexture;
    GLint		m_positionAttributeHandle;
    GLint		m_texCoordAttributeHandle;
    GLint		m_samplerHandle;
    GLint		m_matrixHandle;

public:
    TextureShader();
    virtual ~TextureShader();

    virtual void Link();
    virtual void Setup(const GraphicsSprite& sprite) const;

//    void			SetTexture(Texture2D* pTexture);
//    Texture2D*		GetTexture();
};

//inline void TextureShader::SetTexture(Texture2D* pTexture)
//{
//    m_pTexture = pTexture;
//}
//
//inline Texture2D* TextureShader::GetTexture()
//{
//    return m_pTexture;
//}

#endif // !RENDERER_TEXTURESHADER_H
