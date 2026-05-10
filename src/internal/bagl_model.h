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
  BaglMesh* mesh;
  BaglMaterial* material;
  BaglTransformConfig transformConfig;
  float transform[16];

  bool ownsMesh;
  bool ownsMaterial;
  bool isTransformDirty;
} BaglModel;

#endif