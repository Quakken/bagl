#include "bagl/bagl.h"

#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* realloc, NULL */

#include "bagl/shader.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "internal/bagl_window.h" /* BaglWindow */
#include "internal/bagl_state.h"  /* BaglState */
#include "internal/utils.h"       /* baglLog */

/* Forward */

/* Initializes a BaglState based on configuration data */
static void baglInitState(const BaglConfig* config, BaglState* state);

/* Default function used by a bagl state to log a message. */
static void baglLogFnDefault(BaglLogLevel level, const char* message);

/* Converts a log level to a string. */
static const char* baglLogLevelToStr(BaglLogLevel level);

/* Default configuration used if no alternative is provided */
const static BaglConfig BAGL_CONFIG_DEFAULT = {
    .reallocFn = &realloc,
    .logFn = &baglLogFnDefault,
    .windowConfig = NULL,
};

/* Source code for the fullscreen vertex shader */
const static char* BAGL_FULLSCREEN_VS_SOURCE =
    "#version 410 core                    \n"
    "layout(location = 0) out vec2 uv;    \n"
    "void main() {                        \n"
    "  vec2 pos = vec2(                   \n"
    "    (gl_VertexID == 1) ? 3.0 : -1.0, \n"
    "    (gl_VertexID == 2) ? 3.0 : -1.0  \n"
    "  );                                 \n"
    "  uv = pos * 0.5 + 0.5;              \n"
    "  gl_Position = vec4(pos, 0.0, 1.0); \n"
    "}                                      ";

/* Source code for the model vertex shader */
const static char* BAGL_MODEL_VS_SOURCE =
    "#version 420 core                                                      \n"
    "layout(location = 0) in vec3 position;                                 \n"
    "layout(location = 1) in vec3 normal;                                   \n"
    "layout(location = 2) in vec2 texCoords;                                \n"
    "out VsOut {                                                            \n"
    "  vec2 texCoords;                                                      \n"
    "} vsOut;                                                               \n"
    "layout(std140, binding = 0) uniform Camera {                           \n"
    "  mat4 view;                                                           \n"
    "  mat4 projection;                                                     \n"
    "} camera;                                                              \n"
    "uniform mat4 model;                                                    \n"
    "void main() {                                                          \n"
    "  gl_Position = camera.projection * camera.view * model * vec4(position, "
    "                  1.0);                                                \n"
    "  vsOut.texCoords = texCoords;                                         \n"
    "};                                                                       ";

/* Source code for the textured model fragment shader */
const static char* BAGL_MODEL_TEXTURED_FS_SOURCE =
    "#version 420 core                                              \n"
    "out vec4 color;                                                \n"
    "in VsOut {                                                     \n"
    "  vec2 texCoords;                                              \n"
    "} vsOut;                                                       \n"
    "layout(std140, binding = 1) uniform Material {                 \n"
    "  vec3 ambient;                                                \n"
    "  vec3 diffuse;                                                \n"
    "  vec3 specular;                                               \n"
    "  float specularExponent;                                      \n"
    "} material;                                                    \n"
    "uniform sampler2D ambientMaps[];                               \n"
    "uniform sampler2D diffuseMaps[];                               \n"
    "uniform sampler2D specularMaps[];                              \n"
    "uniform sampler2D normalMaps[];                                \n"
    "void main() {                                                  \n"
    "  vec3 diffuse = texture(diffuseMaps[0], vsOut.texCoords).rgb; \n"
    "  color = vec4(diffuse + material.ambient, 1.0);               \n"
    "}                                                                ";

/* Source code for the untextured (colored) model fragment shader */
const static char* BAGL_MODEL_COLORED_FS_SOURCE =
    "#version 420 core                                  \n"
    "out vec4 color;                                    \n"
    "in VsOut {                                         \n"
    "  vec2 texCoords;                                  \n"
    "} vsOut;                                           \n"
    "layout(std140, binding = 1) uniform Material {     \n"
    "  vec3 ambient;                                    \n"
    "  vec3 specular;                                   \n"
    "  float specularExponent;                          \n"
    "} material;                                        \n"
    "layout(binding = 0) uniform sampler2D diffuseMap;  \n"
    "layout(binding = 1) uniform sampler2D specularMap; \n"
    "layout(binding = 2) uniform sampler2D normalMap;   \n"
    "void main() {                                      \n"
    "  color = vec4(material.ambient, 1.0);             \n"
    "}                                                    ";

/* Definitions */

void baglInit() {
  /* Initialize dependencies */
  glfwInit();
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

void baglPollEvents() {
  glfwPollEvents();
}

void baglDestroyState(BaglState** state) {
  /* Ensure state is valid */
  if (!state || !(*state)) {
    return;
  }

  BaglState* s = *state;

  /* Destroy window */
  baglDestroyWindow(s, &s->window);

  /* Destroy empty VAO */
  glDeleteVertexArrays(1, &s->emptyVAO);

  /* Destroy shaders */
  baglDestroyShaderStage(s, &s->fullscreenVS);
  baglDestroyShaderStage(s, &s->modelVS);
  baglDestroyShader(s, &s->modelTexturedShader);
  baglDestroyShader(s, &s->modelColoredShader);

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

  /* Create window */
  state->window = baglCreateWindow(state, config->windowConfig);

  /* Create empty VAO */
  glGenVertexArrays(1, &state->emptyVAO);
  /* Compile fullscreen vertex stage */
  state->fullscreenVS = baglCompileShaderStage(state, BAGL_STAGE_VERTEX,
                                               BAGL_FULLSCREEN_VS_SOURCE);
  state->modelVS =
      baglCompileShaderStage(state, BAGL_STAGE_VERTEX, BAGL_MODEL_VS_SOURCE);

  BaglShaderStage* texturedFS = baglCompileShaderStage(
      state, BAGL_STAGE_FRAGMENT, BAGL_MODEL_TEXTURED_FS_SOURCE);
  BaglShaderConfig shaderConfig = {
      .vertex = state->modelVS,
      .fragment = texturedFS,
  };
  state->modelTexturedShader = baglCreateShader(state, &shaderConfig);
  BaglShaderStage* coloredFS = baglCompileShaderStage(
      state, BAGL_STAGE_FRAGMENT, BAGL_MODEL_COLORED_FS_SOURCE);
  shaderConfig.fragment = coloredFS;
  state->modelColoredShader = baglCreateShader(state, &shaderConfig);
  baglDestroyShaderStage(state, &texturedFS);
  baglDestroyShaderStage(state, &coloredFS);
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