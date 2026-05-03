/**
 * @file bagl_window.h
 * @authors quak
 * @brief Defines the bagl window object and internal window management
 * utilities.
 */

#ifndef BAGL_BAGL_WINDOW_H
#define BAGL_BAGL_WINDOW_H

#include "bagl/window.h" /* BaglWindow, BaglWindowConfig */

typedef struct GLFWwindow GLFWwindow;

typedef struct BaglWindow {
  GLFWwindow* window;

  /* Base width/height that the window will default to when changing modes */
  int baseWidth;
  int baseHeight;
} BaglWindow;

/**
 * @brief Creates a window.
 * @param state Pointer to the bagl state.
 * @param config Configuration to use when creating the window, or NULL to use
 * default configuration.
 * @return Pointer to the newly created window, or NULL if window could not be
 * created.
 */
BaglWindow* baglCreateWindow(const BaglState* state,
                             const BaglWindowConfig* config);

/**
 * @brief Destroys a window.
 * @param state Pointer to the window's state.
 * @param window Pointer to the window to destroy.
 */
void baglDestroyWindow(const BaglState* state, BaglWindow** window);

#endif