/**
 * @file bagl_state.h
 * @authors quak
 * @brief Defines the bagl state object, which is used throughout the library to
 * store the data required to operate.
 */

#ifndef BAGL_BAGL_STATE_H
#define BAGL_BAGL_STATE_H

#include "bagl/bagl.h"   /* BaglReallocateFn, BaglLogFn, BaglState */
#include "bagl/window.h" /* BaglWindow */

struct BaglState {
  BaglReallocFn reallocFn;
  BaglLogFn logFn;

  BaglWindow* window;
};

#endif