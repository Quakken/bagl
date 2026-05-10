/**
 * @file bagl_state.h
 * @authors quak
 * @brief Defines the bagl state object, which is used throughout the library to
 * store the data required to operate.
 */

#ifndef BAGL_BAGL_STATE_H
#define BAGL_BAGL_STATE_H

#include "bagl/bagl.h" /* BaglReallocateFn, BaglLogFn, BaglState */

typedef struct BaglWindow BaglWindow;
typedef struct BaglShaderStage BaglShaderStage;
typedef struct BaglShader BaglShader;
typedef unsigned int GLuint;

struct BaglState {
  BaglReallocFn reallocFn;
  BaglLogFn logFn;
  BaglWindow* window;

  /* Empty VAO, used when rendering fullscreen triangles */
  GLuint emptyVAO;

  /* Default shaders/stages */
  BaglShaderStage* fullscreenVS;
  BaglShaderStage* modelVS;
  BaglShader* modelTexturedShader;
  BaglShader* modelColoredShader;
};

#endif