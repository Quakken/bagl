/**
 * @file bagl_shader.h
 * @authors quak
 * @brief Defines the BaglShader and BaglShaderStage objects.
 */

#ifndef BAGL_BAGL_SHADER_H
#define BAGL_BAGL_SHADER_H

#include "bagl/shader.h" /* BaglStageType */

typedef unsigned int GLuint;

/* Uniform buffer index to bind matrices to */
const static int BAGL_MATRIX_BINDING = 0;
/* Uniform buffer index to bind materials to */
const static int BAGL_MATERIAL_BINDING = 1;
/* Uniform buffer index to bind lights to */
const static int BAGL_LIGHTS_BINDING = 2;

/* Texture unit reserved for diffuse textures */
const static int BAGL_DIFFUSE_TEXTURE_UNIT = 0;
/* Texture unit reserved to specular textures */
const static int BAGL_SPECULAR_TEXTURE_UNIT = 1;
/* Texture unit reserved for normal textures */
const static int BAGL_NORMAL_TEXTURE_UNIT = 2;
/* First texture unit reserved for color textures */
const static int BAGL_COLOR_TEXTURE_UNIT = 3;

/* Name of the model matrix uniform in shaders */
const static char* BAGL_MODEL_UNIFORM_NAME = "model";

typedef struct BaglShaderStage {
  BaglStageType type;
  GLuint shader;
} BaglShaderStage;

typedef struct BaglShader {
  GLuint program;
} BaglShader;

#endif