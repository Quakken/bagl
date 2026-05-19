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
  /* Allocate the mesh/materials */
  model->meshes = state->reallocFn(NULL, sizeof(BaglMesh*));
  if (!model->meshes) {
    baglLog(state, ERROR, "Could not allocate meshes (in baglCreateModel)");
    state->reallocFn(model, 0);
    return NULL;
  }
  model->materials = state->reallocFn(NULL, sizeof(BaglMaterial*));
  if (!model->materials) {
    baglLog(state, ERROR, "Could not allocate materials (in baglCreateModel)");
    state->reallocFn(model->meshes, 0);
    state->reallocFn(model, 0);
    return NULL;
  }

  /* Initialize members */
  model->meshes[0] = mesh;
  model->ownsMeshes = false;
  if (material) {
    model->materials[0] = material;
    model->ownsMaterials = false;
  } else {
    model->materials[0] = baglCreateMaterial(state, NULL);
    model->ownsMaterials = true;
  }
  model->numMeshes = 1;
  model->capMeshes = 1;
  model->numMaterials = 1;
  model->capMaterials = 1;
  memset(&model->transformConfig, 0, sizeof(model->transformConfig));
  model->isTransformDirty = true;

  baglLog(state, INFO, "Model created");
  return model;
}

BaglModel* baglLoadOBJ(BaglState* state, const char* filename) {
  /* TODO */
  return NULL;
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

  if (m->ownsMeshes) {
    for (size_t i = 0; i < m->numMeshes; ++i) {
      BaglMesh* mesh = m->meshes[i];
      baglDestroyMesh(state, &mesh);
    }
  }
  if (m->ownsMaterials) {
    for (size_t i = 0; i < m->numMaterials; ++i) {
      BaglMaterial* material = m->materials[i];
      baglDestroyMaterial(state, &material);
    }
  }

  state->reallocFn(m->meshes, 0);
  state->reallocFn(m->materials, 0);
  state->reallocFn(m, 0);
  *model = NULL;
  baglLog(state, INFO, "Model destroyed");
}