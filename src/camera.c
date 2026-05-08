#include "bagl/camera.h"

#include <stdlib.h>
#include <math.h>

#include "glad/glad.h"

#include "internal/bagl_state.h"
#include "internal/bagl_camera.h"
#include "internal/matrix.h"
#include "internal/utils.h"

const static BaglCameraConfig BAGL_DEFAULT_CAMERA_CONFIG = {
    .fov = 60.0f,
    .aspect = 16.0f / 9.0f,
    .nearPlane = 0.1f,
    .farPlane = 100.0f,
};

BaglCamera* baglCreateCamera(BaglState* state, const BaglCameraConfig* config) {
  if (!state) {
    return NULL;
  }
  if (!config) {
    config = &BAGL_DEFAULT_CAMERA_CONFIG;
  }

  /* Allocate the camera */
  BaglCamera* camera = state->reallocFn(NULL, sizeof(BaglCamera));
  if (!camera) {
    baglLog(state, ERROR, "Could not allocate camera (in baglCreateCamera)");
    return NULL;
  }
  camera->isViewDirty = true;
  camera->position.x = 0;
  camera->position.y = 0;
  camera->position.z = 0;
  camera->rotation.x = 0;
  camera->rotation.y = 0;
  camera->rotation.z = 0;

  /* Initialize uniform buffers */
  glGenBuffers(1, &camera->ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, camera->ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(BaglCameraLayout), NULL,
               GL_DYNAMIC_DRAW);

  /* Calculate projection matrix and store in UBO */
  BaglCameraLayout* layout = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
  if (!layout) {
    baglLog(state, ERROR,
            "Couldn't map camera uniform buffer (in baglCreateCamera)");
    glDeleteBuffers(1, &camera->ubo);
    state->reallocFn(camera, 0);
    return NULL;
  }
  baglGenPerspective(layout->projection, config->fov, config->aspect,
                     config->nearPlane, config->farPlane);
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  baglLog(state, INFO, "Camera created");
  return camera;
}

void baglSetCameraPosition(BaglState* state,
                           BaglCamera* camera,
                           float x,
                           float y,
                           float z) {
  if (!state || !camera) {
    return;
  }
  camera->position.x = x;
  camera->position.y = y;
  camera->position.z = z;
  camera->isViewDirty = true;
}

void baglSetCameraRotation(BaglState* state,
                           BaglCamera* camera,
                           float x,
                           float y,
                           float z) {
  if (!state || !camera) {
    return;
  }
  camera->rotation.x = fmodf(x, 360.0f);
  camera->rotation.y = fmodf(y, 360.0f);
  camera->rotation.z = fmodf(z, 360.0f);
  camera->isViewDirty = true;
}

void baglDestroyCamera(BaglState* state, BaglCamera** camera) {
  if (!state || !camera || !(*camera)) {
    return;
  }
  BaglCamera* c = *camera;

  glDeleteBuffers(1, &c->ubo);

  state->reallocFn(c, 0);
  *camera = NULL;
  baglLog(state, INFO, "Camera destroyed");
}

void baglUpdateCameraMatrices(BaglState* state, BaglCamera* camera) {
  if (!camera) {
    return;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, camera->ubo);
  BaglCameraLayout* layout = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
  if (!layout) {
    baglLog(state, WARNING,
            "Couldn't map camera uniform buffer (in baglUpdateCameraMatrices)");
    return;
  }
  BaglTransformConfig config = {
      .position = {camera->position.x, camera->position.y, camera->position.z},
      .rotation = {camera->rotation.x, camera->rotation.y, camera->rotation.z},
      .scale = {1.0f, 1.0f, 1.0f},
  };
  baglGenCamView(layout->view, &config);
  glUnmapBuffer(GL_UNIFORM_BUFFER);
  camera->isViewDirty = false;
}