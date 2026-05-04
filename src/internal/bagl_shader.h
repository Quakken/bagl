/**
 * @file bagl_shader.h
 * @authors quak
 * @brief Defines the BaglShader and BaglShaderStage objects.
 */

#ifndef BAGL_BAGL_SHADER_H
#define BAGL_BAGL_SHADER_H

#include "bagl/shader.h" /* BaglStageType */

typedef unsigned int GLuint;

typedef struct BaglShaderStage {
  BaglStageType type;
  GLuint shader;
} BaglShaderStage;

typedef struct BaglShader {
  GLuint program;
} BaglShader;

#endif