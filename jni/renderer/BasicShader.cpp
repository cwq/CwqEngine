#include "BasicShader.h"

BasicShader::BasicShader()
{
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

void BasicShader::Link()
{
    Shader::Link();

    m_positionAttributeHandle = glGetAttribLocation(m_programId, "a_vPosition");
}

void BasicShader::Setup(GraphicsSprite& sprite)
{
    Shader::Setup(sprite);

//    auto geometry = sprite.getGeometry();
//
//    glVertexAttribPointer(
//                          m_positionAttributeHandle,
//                          geometry.GetNumVertexPositionElements(),
//                          GL_FLOAT,
//                          GL_FALSE,
//                          geometry.GetVertexStride(),
//                          geometry.GetVertexBuffer());
//    glEnableVertexAttribArray(m_positionAttributeHandle);
}
