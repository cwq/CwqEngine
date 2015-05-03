#ifndef RENDERER_SHADER_H
#define RENDERER_SHADER_H

#include <string>
#include "platform/CwqGL.h"

#include "GraphicsSprite.h"

class Shader
{
private:
    static GLuint LoadShader(GLenum shaderType, const std::string& shaderCode);

protected:
    GLuint			m_vertexShaderId;
    GLuint			m_fragmentShaderId;
    GLint			m_programId;

    std::string		m_vertexShaderCode;
    std::string		m_fragmentShaderCode;

    bool			m_isLinked;

public:
    Shader();
    virtual ~Shader();

    virtual bool link();
    virtual void setup(const GraphicsSprite& sprite, const GLfloat *mvpMatrix) const;

    bool isLinked() const { return m_isLinked; }
};


#endif // !RENDERER_SHADER_H
