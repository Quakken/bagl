#include "bagl_window.h"

#include <stdio.h>
#include <stdlib.h> /* NULL */

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "bagl/window.h" /* BaglWindowConfig */
#include "bagl_state.h"  /* BaglState */
#include "utils.h"       /* baglLog */

/* Forward */

/**
 * @brief Creates a GLFw WINDOW.
 * @param config Configuration to create window with.
 * @return Pointer to the GLFW window, or NULL if window could not be created.
 */
static GLFWwindow* baglCreateGLFWWindow(const BaglWindowConfig* config);

/**
 * @brief Callback invoked when a GLFW window's framebuffer is resized.
 * @param window GLFW window that was resized.
 * @param width New framebuffer width.
 * @param height New framebuffer height.
 */
static void baglFramebufferResizeCallback(GLFWwindow* window,
                                          int width,
                                          int height);

const static BaglWindowConfig BAGL_DEFAULT_WINDOW_CONFIG = {
    .title = "Bagl",
    .width = 1280,
    .height = 720,
    .mode = BAGL_WINDOW_MODE_WINDOWED,
};

BaglWindow* baglCreateWindow(const BaglState* state,
                             const BaglWindowConfig* config) {
  if (!state) {
    return NULL;
  }
  /* Ensure configuration is valid */
  if (!config) {
    config = &BAGL_DEFAULT_WINDOW_CONFIG;
  }

  /* Allocate the window */
  BaglWindow* window = state->reallocFn(NULL, sizeof(BaglWindow));
  if (!window) {
    baglLog(state, ERROR, "Could not allocate window (in baglCreateWindow)");
    return NULL;
  }

  /* Try to create the GLFW window  */
  window->window = baglCreateGLFWWindow(config);
  if (!window->window) {
    baglLog(state, ERROR, "Unable to create GLFW window (in baglCreateWindow)");
    state->reallocFn(window, 0);
    return NULL;
  }

  /* Initialize other members */
  window->baseWidth = config->width;
  window->baseHeight = config->height;

  baglLog(state, INFO, "Window created");
  return window;
}

void baglDestroyWindow(const BaglState* state, BaglWindow** window) {
  if (!state || !window || !(*window)) {
    return;
  }

  BaglWindow* w = *window;

  /* Destroy GLFW window */
  glfwDestroyWindow(w->window);

  /* Free the window data */
  state->reallocFn(w, 0);

  baglLog(state, INFO, "Window destroyed");
  *window = NULL;
}

static GLFWwindow* baglCreateGLFWWindow(const BaglWindowConfig* config) {
  /* Set GLFW flags */
  GLFWmonitor* monitor = (config->mode == BAGL_WINDOW_MODE_WINDOWED)
                             ? NULL
                             : glfwGetPrimaryMonitor();
  if (config->mode == BAGL_WINDOW_MODE_BORDERLESS) {
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
  }
  /* Initialize the OpenGL context */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  /* Create the window  */
  GLFWwindow* window = glfwCreateWindow(config->width, config->height,
                                        config->title, monitor, NULL);
  if (!window) {
    return NULL;
  }
  /* Bind the OpenGL context */
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(window);
    return NULL;
  }

  /* Bind callbacks */
  glfwSetFramebufferSizeCallback(window, baglFramebufferResizeCallback);

  return window;
}

static void baglFramebufferResizeCallback(GLFWwindow* window,
                                          int width,
                                          int height) {
  glViewport(0, 0, width, height);
}