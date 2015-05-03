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

GLuint Shader::LoadShader(GLenum shaderType, const std::string& shaderCode)
{
    static const int NUM_SHADERS = 1;

    GLuint shaderId = glCreateShader(shaderType);
    if (shaderId)
    {
        const GLchar* pCode = shaderCode.c_str();
        GLint length = shaderCode.length();

        glShaderSource(shaderId, NUM_SHADERS, &pCode, &length);
        glCompileShader(shaderId);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
        if (compiled == GL_FALSE)
        {
            GLint logLength = 0;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 1)
            {
                char *log = new char[logLength];
                glGetShaderInfoLog(shaderId, logLength, NULL, log);
                LOGE("Error compiling shader: %s", log);
                delete []log;
            }
        }
    }

    return shaderId;
}

bool Shader::link()
{
    m_vertexShaderId = LoadShader(GL_VERTEX_SHADER, m_vertexShaderCode);
    if (!m_vertexShaderId)
        return false;

    m_fragmentShaderId = LoadShader(GL_FRAGMENT_SHADER, m_fragmentShaderCode);
    if (!m_fragmentShaderId)
        return false;

    m_programId = glCreateProgram();
    if (!m_programId)
        return false;

    glAttachShader(m_programId, m_vertexShaderId);
    glAttachShader(m_programId, m_fragmentShaderId);
    glLinkProgram(m_programId);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLint logLength = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            char *log = new char[logLength];
            glGetProgramInfoLog(m_programId, logLength, NULL, log);
            LOGE("Could not link program:\n%s\n", log);
            delete []log;
        }
        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }
    
    m_isLinked = true;
    return true;
}

void Shader::setup(const GraphicsSprite& sprite, const GLfloat *mvpMatrix) const
{
    glUseProgram(m_programId);
}
