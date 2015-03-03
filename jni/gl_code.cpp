/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "math/Mat4.h"
#include "LogHelper.h"

struct V3F_C4B_T2F {
    float vertices[3];
    float colors[4];
    float texCoords[2];
};

enum {
    //! top left
    TL = 0,
    //! bottom leftS
    BL = 1,
    //! top right
    TR = 2,
    //! bottom right
    BR = 3
};

//struct V3F_C4B_T2F_Quad {
    V3F_C4B_T2F items[4];
//};

//V3F_C4B_T2F_Quad mQuad;

GLuint gl_buffer_id;

float vct[36];

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

static const char gVertexShader[] = 
    "attribute vec4 vPosition;\n"
    "uniform mat4 u_Matrix;\n"
    "attribute vec2 a_TextureCoordinates;\n"
    "varying vec2 v_TextureCoordinates;\n"
    "void main() {\n"
    "  v_TextureCoordinates = a_TextureCoordinates;\n"
    "  gl_Position = u_Matrix * vPosition;\n"
    "}\n";

static const char gFragmentShader[] = 
    "precision mediump float;\n"
    "uniform sampler2D u_TextureUnit;\n"
    "varying vec2 v_TextureCoordinates;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(u_TextureUnit, v_TextureCoordinates);\n"
    "}\n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionHandle;
GLuint a_TextureCoordinates;
GLuint u_TextureUnit;
GLuint u_Matrix;

Mat4 mvpMat;
Mat4 vMat;
Mat4 pMat;

bool setupGraphics() {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics()");
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
            gvPositionHandle);
    u_Matrix = glGetUniformLocation(gProgram, "u_Matrix");
    checkGlError("glGetUniformLocation");

    a_TextureCoordinates = glGetAttribLocation(gProgram, "a_TextureCoordinates");
    checkGlError("glGetAttribLocation");
    u_TextureUnit = glGetUniformLocation(gProgram, "u_TextureUnit");
    checkGlError("glGetUniformLocation");

    Mat4::createLookAt(Vec3(0, 0, 1), Vec3(0, 0, 0), Vec3(0, 1, 0), &vMat);

    ////////////////////////
    // 更新相关几何信息
    items[0].vertices[0] = vct[0] = .0f;
    items[0].vertices[1] = vct[1] = 500.0f;
    items[0].vertices[2] = vct[2] = .0f;

    items[0].texCoords[0] = vct[7] = .0f;
    items[0].texCoords[1] = vct[8] = 1.0f;

    items[1].vertices[0] = vct[9] = .0f;
    items[1].vertices[1] = vct[10] = .0f;
    items[1].vertices[2] = vct[11] = .0f;

    items[1].texCoords[0] = vct[16] = .0f;
    items[1].texCoords[1] = vct[17] = .0f;

    items[2].vertices[0] = vct[18] = 500.0f;
    items[2].vertices[1] = vct[19] = 500.0f;
    items[2].vertices[2] = vct[20] = .0f;

    items[2].texCoords[0] = vct[25] = 1.0f;
    items[2].texCoords[1] = vct[26] = 1.0f;

    items[3].vertices[0] = vct[27] = 500.0f;
    items[3].vertices[1] = vct[28] = .0f;
    items[3].vertices[2] = vct[29] = .0f;

    items[3].texCoords[0] = vct[34] = 1.0f;
    items[3].texCoords[1] = vct[35] = .0f;

    glGenBuffers(1, &gl_buffer_id);
    checkGlError("glGenBuffers");

    return true;
}

void surfaceChanged(int w, int h) {
    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    Mat4::createOrthographicOffCenter(0, w, 0, h, -1, 1, &pMat);
    Mat4::multiply(vMat, pMat, &mvpMat);
}

const GLfloat gTriangleVertices[] = { -0.5f, 0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, 0.5f, -0.5f };
const GLfloat gTextureCoordinates[] = { 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f };
const GLfloat mat[] = { 1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f };

void renderFrame(int textureID) {
//    static float grey;
//    grey += 0.01f;
//    if (grey > 1.0f) {
//        grey = 0.0f;
//    }
//    glClearColor(grey, grey, grey, 1.0f);
//    checkGlError("glClearColor");
//    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glBindBuffer(GL_ARRAY_BUFFER,  gl_buffer_id);
    checkGlError("glBindBuffer");
    glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F) * 4, items, GL_DYNAMIC_DRAW);
    checkGlError("glBufferData");

    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));//items
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(a_TextureCoordinates, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));//items[0].texCoords
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(a_TextureCoordinates);
    checkGlError("glEnableVertexAttribArray");

    glUniformMatrix4fv(u_Matrix, 1, GL_FALSE, mvpMat.m);
    checkGlError("glUniformMatrix4fv");

    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, textureID);
    checkGlError("glBindTexture");
    glUniform1i(u_TextureUnit, 0);
    checkGlError("glUniform1i");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    checkGlError("glDrawArrays");
}
