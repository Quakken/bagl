#include "bagl/model.h"

#include <stdbool.h>
#include <string.h>

#include "bagl/mesh.h"
#include "bagl/material.h"

#include "internal/bagl_model.h"
#include "internal/bagl_state.h"
#include "internal/bagl_mesh.h"
#include "internal/bagl_material.h"
#include "internal/utils.h"

BaglModel* baglCreateModel(BaglState* state,
                           BaglMesh* mesh,
                           BaglMaterial* material) {
  if (!state) {
    return NULL;
  }
  if (!mesh) {
    baglLog(state, ERROR, "Mesh cannot be NULL (in baglCreateModel)");
    return NULL;
  }

  /* Allocate the model */
  BaglModel* model = state->reallocFn(NULL, sizeof(BaglModel));
  if (!model) {
    baglLog(state, ERROR, "Could not allocate model (in baglCreateModel)");
    return NULL;
  }

  /* Initialize members */
  model->mesh = mesh;
  model->ownsMesh = false;
  if (material) {
    model->material = material;
    model->ownsMaterial = false;
  } else {
    model->material = baglCreateMaterial(state, NULL);
    model->ownsMaterial = true;
  }
  memset(&model->transformConfig, 0, sizeof(model->transformConfig));
  model->isTransformDirty = true;

  baglLog(state, INFO, "Model created");
  return model;
}

void baglSetModelPosition(BaglState* state,
                          BaglModel* model,
                          float x,
                          float y,
                          float z) {
  if (!state || !model) {
    return;
  }
  model->transformConfig.position.x = x;
  model->transformConfig.position.y = y;
  model->transformConfig.position.z = z;
  model->isTransformDirty = true;
}

void baglSetModelRotation(BaglState* state,
                          BaglModel* model,
                          float x,
                          float y,
                          float z) {
  if (!state || !model) {
    return;
  }
  model->transformConfig.rotation.x = x;
  model->transformConfig.rotation.y = y;
  model->transformConfig.rotation.z = z;
  model->isTransformDirty = true;
}

void baglSetModelScale(BaglState* state,
                       BaglModel* model,
                       float x,
                       float y,
                       float z) {
  if (!state || !model) {
    return;
  }
  model->transformConfig.scale.x = x;
  model->transformConfig.scale.y = y;
  model->transformConfig.scale.z = z;
  model->isTransformDirty = true;
}

void baglDestroyModel(BaglState* state, BaglModel** model) {
  if (!state || !model || !(*model)) {
    return;
  }
  BaglModel* m = *model;

  if (m->ownsMesh) {
    baglDestroyMesh(state, &m->mesh);
  }
  if (m->ownsMaterial) {
    baglDestroyMaterial(state, &m->material);
  }

  state->reallocFn(m, 0);
  *model = NULL;
  baglLog(state, INFO, "Model destroyed");
}