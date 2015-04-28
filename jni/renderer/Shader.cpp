#include "Shader.h"
#include "base/LogHelper.h"

Shader::Shader()
:	m_vertexShaderId(GL_INVALID_VALUE)
,	m_fragmentShaderId(GL_INVALID_VALUE)
,	m_programId(GL_INVALID_VALUE)
{
    m_isLinked = false;
}

Shader::~Shader()
{

}

void Shader::LoadShader(GLuint id, const std::string& shaderCode)
{
    static const int NUM_SHADERS = 1;

    const GLchar* pCode = shaderCode.c_str();
    GLint length = shaderCode.length();

    glShaderSource(id, NUM_SHADERS, &pCode, &length);

    glCompileShader(id);

    GLint compiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        GLint logLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            char *log = new char[logLength];
            glGetShaderInfoLog(id, logLength, NULL, log);
            LOGE("Error compiling shader: %s", log);
            delete []log;
        }
    }

    glAttachShader(m_programId, id);
}

void Shader::Link()
{
    m_programId = glCreateProgram();

    m_vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    LoadShader(m_vertexShaderId, m_vertexShaderCode);

    m_fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    LoadShader(m_fragmentShaderId, m_fragmentShaderCode);

    glLinkProgram(m_programId);
    
    m_isLinked = true;
}

void Shader::Setup(const GraphicsSprite& sprite) const
{
    glUseProgram(m_programId);
}
