#pragma once

#include <stdint.h>

#include "nvui.h"
#include "glad/gl.h"

#define VERTEX_FORMAT_END (VertexFormat){ .size = -1 }

typedef struct VertexFormat
{
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLuint offset;
    GLuint bind;
    GLboolean targetInt;
} VertexFormat;

NVAPI bool CompileShader(const char *shaderScr, GLint len, GLuint *shader, char *logBuffer, size_t bufferSize);
NVAPI bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint *program, char *logBuffer, size_t bufferSize);
NVAPI GLuint CreateSimpleTexture(int width, int height, const uint8_t *image);
NVAPI GLuint CreateVertexArray(VertexFormat formatDesc[]);
