#include "nvui/glutils.h"

#include <stdlib.h>

NVAPI bool CompileShader(const char *shaderScr, GLint len, GLuint *shader, char *logBuffer, size_t bufferSize)
{
    int success;

    glShaderSource(*shader, 1, &shaderScr, &len);
    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(*shader, bufferSize, NULL, logBuffer);
        //printf("Failed to compile Shader: %s\n", logBuffer);
        return false;
    };

    return true;
}

NVAPI bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint *program, char *logBuffer, size_t bufferSize)
{
    int success;

    glAttachShader(*program, vertexShader);
    glAttachShader(*program, fragmentShader);
    glLinkProgram(*program);
    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(*program, bufferSize, NULL, logBuffer);
        //printf("Failed to link Program: %s\n", logBuffer);
        return false;
    }

    return true;
}

NVAPI GLuint CreateSimpleTexture(int width, int height, const uint8_t *image)
{
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

NVAPI GLuint CreateVertexArray(VertexFormat formatDesc[])
{
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    int idx = 0;
    VertexFormat fmt;
    while(true)
    {
        fmt = formatDesc[idx];
        if(fmt.size == -1) break;

        glEnableVertexArrayAttrib(vao, idx);
        if(fmt.type == GL_DOUBLE)
            glVertexArrayAttribLFormat(vao, idx, fmt.size, fmt.type, fmt.offset);
        else if(fmt.targetInt && (fmt.type == GL_BYTE || fmt.type == GL_SHORT || fmt.type == GL_INT || fmt.type == GL_UNSIGNED_BYTE || fmt.type == GL_UNSIGNED_SHORT || fmt.type == GL_UNSIGNED_INT))
            glVertexArrayAttribIFormat(vao, idx, fmt.size, fmt.type, fmt.offset);
        else
            glVertexArrayAttribFormat(vao, idx, fmt.size, fmt.type, fmt.normalized, fmt.offset);
        glVertexArrayAttribBinding(vao, idx, fmt.bind);
        idx++;
    }
    
    return vao;
}
