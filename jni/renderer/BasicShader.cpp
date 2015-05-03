#include "BasicShader.h"

BasicShader::BasicShader()
{
    m_positionAttributeHandle = GL_INVALID_VALUE;

    m_vertexShaderCode =
				"attribute vec4 a_vPosition;        \n"
				"void main(){                       \n"
				"     gl_Position = a_vPosition;	\n"
				"}                                  \n";

    m_fragmentShaderCode =
				"precision mediump float;  						\n"
				"void main(){              						\n"
				"    gl_FragColor = vec4(1.0, 0.2, 0.2, 1.0);   \n"
				"}                         						\n";
}

BasicShader::~BasicShader()
{

}

bool BasicShader::link()
{
    if (Shader::link())
    {
        m_positionAttributeHandle = glGetAttribLocation(m_programId, "a_vPosition");
        return true;
    }
    return false;
}

void BasicShader::setup(const GraphicsSprite& sprite, const GLfloat *mvpMatrix) const
{
    Shader::setup(sprite, mvpMatrix);
}
