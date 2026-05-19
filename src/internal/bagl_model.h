/**
 * @file bagl_model.h
 * @authors quak
 * @brief Defines the BaglModel object.
 */

#ifndef BAGL_BAGL_MODEL_H
#define BAGL_BAGL_MODEL_H

#include <stdbool.h>
#include "matrix.h" /* BaglTransformConfig */

typedef struct BaglMesh BaglMesh;
typedef struct BaglMaterial BaglMaterial;

typedef struct BaglModel {
  BaglMesh** meshes;
  BaglMaterial** materials;
  BaglTransformConfig transformConfig;
  float transform[16];

  size_t numMeshes;
  size_t capMeshes;
  size_t numMaterials;
  size_t capMaterials;

  bool ownsMeshes;
  bool ownsMaterials;
  bool isTransformDirty;
} BaglModel;

#endif