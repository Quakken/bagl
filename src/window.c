#include "bagl/window.h"

#include "GLFW/glfw3.h"

#include "internal/bagl_window.h"
#include "internal/bagl_state.h"

int baglGetWindowWidth(const BaglState* state) {
  if (!state || !state->window) {
    return 0;
  }
  int width;
  glfwGetWindowSize(state->window->window, &width, NULL);
  return width;
}

int baglGetWindowHeight(const BaglState* state) {
  if (!state || !state->window) {
    return 0;
  }
  int height;
  glfwGetWindowSize(state->window->window, NULL, &height);
  return height;
}

int baglGetViewportWidth(const BaglState* state) {
  if (!state || !state->window) {
    return 0;
  }
  int width;
  glfwGetFramebufferSize(state->window->window, &width, NULL);
  return width;
}

int baglGetViewportHeight(const BaglState* state) {
  if (!state || !state->window) {
    return 0;
  }
  int height;
  glfwGetFramebufferSize(state->window->window, NULL, &height);
  return height;
}

bool baglShouldClose(const BaglState* state) {
  if (!state) {
    return 1;
  }
  if (state->window == NULL) {
    return 1;
  }
  return glfwWindowShouldClose(state->window->window);
}

void baglSetWindowMode(const BaglState* state, BaglWindowMode mode) {
  if (!state || !state->window) {
    return;
  }
  GLFWmonitor* monitor =
      (mode == BAGL_WINDOW_MODE_WINDOWED) ? NULL : glfwGetPrimaryMonitor();
  if (mode == BAGL_WINDOW_MODE_BORDERLESS) {
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(state->window->window, monitor, 0, 0, videoMode->width,
                         videoMode->height, videoMode->refreshRate);
  } else {
    glfwSetWindowMonitor(state->window->window, monitor, 0, 0,
                         state->window->baseWidth, state->window->baseHeight,
                         GLFW_DONT_CARE);
  }
}

void* baglGetWrappedWindow(const BaglState* state) {
  if (!state || !state->window) {
    return NULL;
  }
  return state->window->window;
}