/**
 * @file bagl_camera.h
 * @authors quak
 * @brief Defines the BaglCamera object.
 */

#ifndef BAGL_BAGL_CAMERA_H
#define BAGL_BAGL_CAMERA_H

#include <stdbool.h> /* bool */

typedef unsigned int GLuint;
typedef struct BaglState BaglState;

typedef struct BaglCameraLayout {
  float view[16];
  float projection[16];
} BaglCameraLayout;

typedef struct BaglCamera {
  /* Matrices are stored in the UBO */
  /* When camera moves, only the view matrix needs to be recalculated */

  struct {
    float x, y, z;
  } position;
  struct {
    float x, y, z;
  } rotation;
  GLuint ubo;
  bool isViewDirty;
} BaglCamera;

/**
 * @brief Updates a camera's matrices.
 * @param camera Pointer to the camera to update.
 */
void baglUpdateCameraMatrices(BaglState* state, BaglCamera* camera);

#endif