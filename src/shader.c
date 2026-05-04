#include "bagl/shader.h"

#include <stdlib.h> /* NULL */
#include <stdio.h>  /* FILE, fopen, fclose */

#include <glad/glad.h> /* OpenGL */
#include <string.h>

#include "internal/bagl_state.h"
#include "internal/bagl_shader.h"
#include "internal/utils.h"

static GLenum baglGetStageType(BaglStageType type);

static bool baglValidateShaderConfig(BaglState* state,
                                     BaglShaderConfig* config);

BaglShaderStage* baglLoadShaderStage(BaglState* state,
                                     BaglStageType type,
                                     const char* filename) {
  if (!state || !filename) {
    return NULL;
  }

  /* Load shader source */
  FILE* file;
  errno_t error = fopen_s(&file, filename, "rt");
  if (error) {
    baglLog(state, ERROR,
            "Error loading stage source (in baglLoadShaderStage)");
    return NULL;
  }

  /* Get the size of the shader data */
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);

  /* Allocate buffer to store shader contents */
  char* buffer = state->reallocFn(NULL, size + 1);
  if (!buffer) {
    baglLog(state, ERROR,
            "Could not allocate buffer to store stage source (in "
            "baglLoadShaderStage)");
    fclose(file);
    return NULL;
  }
  buffer[size] = '\0';

  /* Read shader contents into the file */
  fseek(file, 0, SEEK_SET);
  size_t bytesRead = fread(buffer, 1, size, file);

  /* Handle errors */
  if (bytesRead != size && ferror(file)) {
    baglLog(state, ERROR,
            "Error reading stage source (in baglLoadShaderStage)");
    fclose(file);
    state->reallocFn(buffer, 0);
    return NULL;
  }
  fclose(file);

  /* Compile the shader */
  BaglShaderStage* stage = baglCompileShaderStage(state, type, buffer);
  state->reallocFn(buffer, 0);

  if (!stage) {
    baglLog(state, ERROR,
            "Couldn't compile shader stage (in baglLoadShaderStage)");
    return NULL;
  }

  baglLog(state, INFO, "Stage loaded");
  return stage;
}

BaglShaderStage* baglCompileShaderStage(BaglState* state,
                                        BaglStageType type,
                                        const char* source) {
  if (!state || !source) {
    return NULL;
  }

  /* Allocate the stage */
  BaglShaderStage* stage = state->reallocFn(NULL, sizeof(BaglShaderStage));
  if (!stage) {
    baglLog(state, ERROR,
            "Could not allocate stage (in baglCompileShaderStage)");
    return NULL;
  }

  /* Compile the shader */
  stage->type = type;
  stage->shader = glCreateShader(baglGetStageType(type));
  glShaderSource(stage->shader, 1, &source, NULL);
  glCompileShader(stage->shader);

  /* Make sure compilation was successful */
  int success;
  glGetShaderiv(stage->shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char errorLog[512];
    glGetShaderInfoLog(stage->shader, 512, NULL, errorLog);
    baglLog(state, ERROR, errorLog);
    glDeleteShader(stage->shader);
    state->reallocFn(stage, 0);
    return NULL;
  }

  baglLog(state, INFO, "Stage compiled");
  return stage;
}

BaglShader* baglCreateShader(BaglState* state, BaglShaderConfig* config) {
  if (!state) {
    return NULL;
  }
  if (!config) {
    baglLog(state, ERROR, "Config cannot be NULL (in baglCreateShader)");
    return NULL;
  }

  /* Validate configuration */
  if (!baglValidateShaderConfig(state, config)) {
    return NULL;
  }

  /* Allocate the shader */
  BaglShader* shader = state->reallocFn(NULL, sizeof(BaglShader));
  if (!shader) {
    baglLog(state, ERROR, "Couldn't allocate shader (in baglCreateShader)");
    return NULL;
  }
  shader->program = glCreateProgram();

  /* Get each shader stage */
  BaglShaderStage* vertex = config->vertex;
  if (!vertex) {
    if (config->vertexFilename) {
      vertex =
          baglLoadShaderStage(state, BAGL_STAGE_VERTEX, config->vertexFilename);
    } else {
      vertex = state->fullscreenVS;
    }
  }
  BaglShaderStage* geometry = config->geometry;
  if (!geometry && config->geometryFilename) {
    geometry = baglLoadShaderStage(state, BAGL_STAGE_GEOMETRY,
                                   config->geometryFilename);
  }
  BaglShaderStage* fragment = config->fragment;
  if (!fragment) {
    fragment = baglLoadShaderStage(state, BAGL_STAGE_FRAGMENT,
                                   config->fragmentFilename);
  }

  /* Link all of the stages to the shader */
  if (vertex) {
    glAttachShader(shader->program, vertex->shader);
  } else {
    baglLog(state, ERROR,
            "Couldn't resolve vertex shader (in baglCreateShader)");
    glDeleteProgram(shader->program);
    state->reallocFn(shader, 0);
  }
  if (geometry) {
    glAttachShader(shader->program, vertex->shader);
  }
  if (fragment) {
    glAttachShader(shader->program, fragment->shader);
  } else {
    baglLog(state, ERROR,
            "Couldn't resolve fragment shader (in baglCreateShader)");
    glDeleteProgram(shader->program);
    state->reallocFn(shader, 0);
  }
  glLinkProgram(shader->program);

  /* Clean up any stages that were created */
  if (!config->vertex && config->vertexFilename) {
    baglDestroyShaderStage(state, &vertex);
  }
  if (!config->geometry && config->geometryFilename) {
    baglDestroyShaderStage(state, &geometry);
  }
  if (!config->fragment && config->fragmentFilename) {
    baglDestroyShaderStage(state, &fragment);
  }

  baglLog(state, INFO, "Shader created");
  return shader;
}

void baglDestroyShaderStage(BaglState* state, BaglShaderStage** stage) {
  if (!state || !stage || !(*stage)) {
    return;
  }
  BaglShaderStage* s = *stage;

  glDeleteShader(s->shader);

  state->reallocFn(s, 0);
  *stage = NULL;
  baglLog(state, INFO, "Stage destroyed");
}

void baglDestroyShader(BaglState* state, BaglShader** shader) {
  if (!state || !shader || !(*shader)) {
    return;
  }
  BaglShader* s = *shader;

  glDeleteProgram(s->program);

  state->reallocFn(s, 0);
  *shader = NULL;
  baglLog(state, INFO, "Shader destroyed");
}

static bool baglValidateShaderConfig(BaglState* state,
                                     BaglShaderConfig* config) {
  if (!config->fragment && !config->fragmentFilename) {
    baglLog(state, ERROR, "A fragment stage is required (in baglCreateShader)");
    return false;
  }
  if (config->vertex && config->vertexFilename) {
    baglLog(state, WARNING,
            "Multiple vertex stages provided (in baglCreateShader)");
  }
  if (config->geometry && config->geometryFilename) {
    baglLog(state, WARNING,
            "Multiple geometry stages provided (in baglCreateShader)");
  }
  if (config->fragment && config->fragmentFilename) {
    baglLog(state, WARNING,
            "Multiple fragment stages provided (in baglCreateShader)");
  }
  return true;
}

static GLenum baglGetStageType(BaglStageType type) {
  switch (type) {
    case BAGL_STAGE_VERTEX:
      return GL_VERTEX_SHADER;
    case BAGL_STAGE_GEOMETRY:
      return GL_GEOMETRY_SHADER;
    case BAGL_STAGE_FRAGMENT:
      return GL_FRAGMENT_SHADER;
  }
}