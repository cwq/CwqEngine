#include "TextureShader.h"
#include "Texture2D.h"

TextureShader::TextureShader()
{
    m_positionAttributeHandle = GL_INVALID_VALUE;
    m_texCoordAttributeHandle = GL_INVALID_VALUE;
    m_samplerHandle = GL_INVALID_VALUE;
    m_matrixHandle = GL_INVALID_VALUE;

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

bool TextureShader::link()
{
    if (Shader::link())
    {
        m_positionAttributeHandle	= glGetAttribLocation(m_programId, "a_vPosition");
        m_texCoordAttributeHandle	= glGetAttribLocation(m_programId, "a_texCoord");
        m_samplerHandle				= glGetUniformLocation(m_programId, "s_texture");
        m_matrixHandle              = glGetUniformLocation(m_programId, "u_Matrix");
        return true;
    }
    return false;
}

void TextureShader::setup(const GraphicsSprite& sprite, const GLfloat *mvpMatrix) const
{
    Shader::setup(sprite, mvpMatrix);

    glEnableVertexAttribArray(m_positionAttributeHandle);
    glEnableVertexAttribArray(m_texCoordAttributeHandle);
    // vertices
    glVertexAttribPointer(m_positionAttributeHandle, 3, GL_FLOAT, GL_FALSE, V3F_C4F_T2F_SIZE, (GLvoid*) Vertex3F_OFFSET);
    // tex coords
    glVertexAttribPointer(m_texCoordAttributeHandle, 2, GL_FLOAT, GL_FALSE, V3F_C4F_T2F_SIZE, (GLvoid*) Tex2F_OFFSET);

    glUniformMatrix4fv(m_matrixHandle, 1, GL_FALSE, mvpMatrix);

    if (sprite.getTexture())
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite.getTexture()->getTextureID());
        glUniform1i(m_samplerHandle, 0);
    }
}
