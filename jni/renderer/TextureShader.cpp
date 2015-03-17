#include "TextureShader.h"

#include <cassert>

#include "Shader.h"
#include "../Texture2D.h"


TextureShader::TextureShader()
//:	m_pTexture(NULL)
{
    m_vertexShaderCode =
				"attribute vec4 a_vPosition;        \n"
				"attribute vec2 a_texCoord;         \n"
				"varying   vec2 v_texCoord;         \n"
				"uniform   mat4 u_Matrix;           \n"
				"void main(){                       \n"
				"    gl_Position = u_Matrix * a_vPosition;		\n"
				"    v_texCoord = a_texCoord;		\n"
				"}                                  \n";

    m_fragmentShaderCode =
				"precision highp float;    							\n"
				"varying vec2 v_texCoord;								\n"
				"uniform sampler2D s_texture;							\n"
				"void main(){                							\n"
				"    gl_FragColor = texture2D(s_texture, v_texCoord);   \n"
				"}                         								\n";
}

TextureShader::~TextureShader()
{

}

void TextureShader::Link()
{
    Shader::Link();

    m_positionAttributeHandle	= glGetAttribLocation(m_programId, "a_vPosition");
    m_texCoordAttributeHandle	= glGetAttribLocation(m_programId, "a_texCoord");
    m_samplerHandle				= glGetUniformLocation(m_programId, "s_texture");
    m_matrixHandle              = glGetUniformLocation(m_programId, "u_Matrix");
}

void TextureShader::Setup(GraphicsSprite& sprite)
{
    assert(sprite.getTexture());
    //auto geometry = sprite.getGeometry();
    if (sprite.getTexture())
    {
        Shader::Setup(sprite);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite.getTexture()->getTextureID());
        glUniform1i(m_samplerHandle, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//        glVertexAttribPointer(
//                              m_positionAttributeHandle,
//                              geometry.GetNumVertexPositionElements(),
//                              GL_FLOAT,
//                              GL_FALSE,
//                              geometry.GetVertexStride(),
//                              geometry.GetVertexBuffer());
//        glEnableVertexAttribArray(m_positionAttributeHandle);
//
//        glVertexAttribPointer(
//                              m_texCoordAttributeHandle,
//                              geometry.GetNumTexCoordElements(),
//                              GL_FLOAT,
//                              GL_FALSE,
//                              geometry.GetVertexStride(),
//                              geometry.GetTexCoordVertexBuffer());
//        glEnableVertexAttribArray(m_texCoordAttributeHandle);
    }
}
