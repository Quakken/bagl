/**
 * @file bagl_shader.h
 * @authors quak
 * @brief Defines the BaglShader and BaglShaderStage objects.
 */

#ifndef BAGL_BAGL_SHADER_H
#define BAGL_BAGL_SHADER_H

#include "bagl/shader.h" /* BaglStageType */

typedef unsigned int GLuint;

/* Name of the model matrix uniform in shaders */
const static char* BAGL_MODEL_UNIFORM_NAME = "model";
/* Name of the material uniform block in shaders */
const static char* BAGL_MATERIAL_UNIFORM_NAME = "Material";
/* Name of the camera uniform block in shaders */
const static char* BAGL_CAMERA_UNIFORM_NAME = "Camera";

/* Binding point used by the material uniform block */
const static unsigned int BAGL_MATERIAL_UNIFORM_BIND_POINT = 0;
/* Binding point used by the camera uniform block */
const static unsigned int BAGL_CAMERA_UNIFORM_BIND_POINT = 1;

/* Name of the uniform that stores time */
const static char* BAGL_TIME_UNIFORM_NAME = "time";
/* Name of the uniform array that holds color data for baglProcess operations */
const static char* BAGL_PROCESS_TEXTURE_NAME = "colorMaps[%d]";
/* Name of the uniform array that holds ambient texture data */
const static char* BAGL_AMBIENT_UNIFORM_NAME = "ambientMaps[%d]";
/* Name of the uniform array that holds diffuse texture data */
const static char* BAGL_DIFFUSE_UNIFORM_NAME = "diffuseMaps[%d]";
/* Name of the uniform array that holds specular texture data */
const static char* BAGL_SPECULAR_UNIFORM_NAME = "specularMaps[%d]";
/* Name of the uniform array that holds normal texture data */
const static char* BAGL_NORMAL_UNIFORM_NAME = "normalMaps[%d]";

typedef struct BaglShaderStage {
  BaglStageType type;
  GLuint shader;
} BaglShaderStage;

typedef struct BaglShader {
  GLuint program;
} BaglShader;

bool baglSetUniformBlock(BaglState* state,
                         BaglShader* shader,
                         const char* name,
                         unsigned int bindingPoint);

#endif