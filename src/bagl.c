#include "bagl/bagl.h"

#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* realloc, NULL */

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "internal/bagl_window.h" /* BaglWindow */
#include "internal/bagl_state.h"  /* BaglState */
#include "internal/utils.h"       /* baglLog */

/* Forward */

/* Initializes a BaglState based on configuration data */
static void baglInitState(const BaglConfig* config, BaglState* state);

/* Default function used by a bagl state to log a mesage. */
static void baglLogFnDefault(BaglLogLevel level, const char* message);

/* Converts a log level to a string. */
static const char* baglLogLevelToStr(BaglLogLevel level);

/* Default configuration used if no alternative is provided */
const static BaglConfig BAGL_CONFIG_DEFAULT = {
    .reallocFn = &realloc,
    .logFn = &baglLogFnDefault,
    .windowConfig = NULL,
};

/* Definitions */

void baglInit() {
  /* Initialize dependencies */
  glfwInit();
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

BaglState* baglCreateState(const BaglConfig* config) {
  /* Ensure there is a valid configuration to read from */
  if (!config) {
    config = &BAGL_CONFIG_DEFAULT;
  }

  /* Allocate the state */
  BaglState* state;
  if (config->reallocFn) {
    state = config->reallocFn(NULL, sizeof(BaglState));
  } else {
    state = realloc(NULL, sizeof(BaglState));
  }
  if (!state) {
    /* Allocation failed */
    if (config->logFn) {
      config->logFn(BAGL_LOG_ERROR,
                    "Could not allocate bagl state (in baglCreateState)");
    }
    return NULL;
  }

  /* Initialize memory */
  baglInitState(config, state);

  baglLog(state, INFO, "State created");
  return state;
}

void baglDestroyState(BaglState** state) {
  /* Ensure state is valid */
  if (!state || !(*state)) {
    return;
  }

  BaglState* s = *state;

  /* Destroy window */
  baglDestroyWindow(s, &s->window);

  /* Free state memory */
  baglLog(s, INFO, "Destroying state");
  s->reallocFn(s, 0);

  *state = NULL;
}

void baglTerminate() {
  /* Terminate GLFW */
  glfwTerminate();
}

static void baglInitState(const BaglConfig* config, BaglState* state) {
  /* Bind callbacks */
  state->logFn = config->logFn;
  state->reallocFn =
      (config->reallocFn) ? config->reallocFn : BAGL_CONFIG_DEFAULT.reallocFn;
  state->window = baglCreateWindow(state, config->windowConfig);
}

static void baglLogFnDefault(BaglLogLevel level, const char* message) {
  /* Send errors to stderr */
  if (level >= BAGL_LOG_WARNING) {
    const char* levelStr = baglLogLevelToStr(level);
    fprintf(stderr, "[bagl] %s: %s\n", levelStr, message);
  }
}

static const char* baglLogLevelToStr(BaglLogLevel level) {
  switch (level) {
    case BAGL_LOG_INFO:
      return "INFO";
    case BAGL_LOG_STANDARD:
      return "STANDARD";
    case BAGL_LOG_WARNING:
      return "WARNING";
    case BAGL_LOG_ERROR:
      return "ERROR";
    case BAGL_LOG_FATAL:
      return "FATAL";
  }
}