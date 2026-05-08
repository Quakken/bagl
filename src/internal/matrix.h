/**
 * @file matrix.h
 * @authors quak
 * @brief Utilities for generating matrices.
 */

#ifndef BAGL_MATRIX_H
#define BAGL_MATRIX_H

/**
 * @brief Generates a perspective matrix.
 * @param matrix Matrix to store output to.
 * @param fov Field of view.
 * @param aspect Aspect ratio.
 * @param near Near plane distance.
 * @param far Far plane distance.
 */
void baglGenPerspective(float matrix[16],
                        float fov,
                        float aspect,
                        float near,
                        float far);

typedef struct BaglTransformConfig {
  struct {
    float x, y, z;
  } position;
  struct {
    float x, y, z;
  } rotation;
  struct {
    float x, y, z;
  } scale;
} BaglTransformConfig;

/**
 * @brief Generates a transformation matrix.
 * @param matrix Matrix to store output to.
 * @param config Configuration to use to generate the transform.
 */
void baglGenTransform(float matrix[16], const BaglTransformConfig* config);

/**
 * @brief Generates a view matrix for a camera.
 * @param matrix Matrix to store output to.
 * @param config Camera position and rotation (scale is unused).
 */
void baglGenCamView(float matrix[16], const BaglTransformConfig* config);

#endif