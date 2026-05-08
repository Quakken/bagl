#include "matrix.h"

#define _USE_MATH_DEFINES
#include <math.h>

/*
 * Calculates the matrix index at the given x and y coordinates (column major)
 */
#define idx(x, y) x * 4 + y

/* Converts from degrees to radians */
#define rad(deg) (deg * M_PI / 180.0f)

/* Calculates the dot product between two vectors */
static float dot(float x1, float y1, float z1, float x2, float y2, float z2) {
  return x1 * x2 + y1 * y2 + z1 * z2;
}

void baglGenPerspective(float matrix[16],
                        float fov,
                        float aspect,
                        float near,
                        float far) {
  if (!matrix) {
    return;
  }
  /* Formula taken from https://ogldev.org/www/tutorial12/tutorial12.html */
  float halfTanFOV = tanf(rad(fov) / 2.0f);

  matrix[idx(0, 0)] = 1.0f / (aspect * halfTanFOV);
  matrix[idx(0, 1)] = 0.0f;
  matrix[idx(0, 2)] = 0.0f;
  matrix[idx(0, 3)] = 0.0f;

  matrix[idx(1, 0)] = 0.0f;
  matrix[idx(1, 1)] = 1.0f / halfTanFOV;
  matrix[idx(1, 2)] = 0.0f;
  matrix[idx(1, 3)] = 0.0f;

  matrix[idx(2, 0)] = 0.0f;
  matrix[idx(2, 1)] = 0.0f;
  matrix[idx(2, 2)] = (-near - far) / (near - far);
  matrix[idx(2, 3)] = 1.0f;

  matrix[idx(3, 0)] = 0.0f;
  matrix[idx(3, 1)] = 0.0f;
  matrix[idx(3, 2)] = (2 * near * far) / (near - far);
  matrix[idx(3, 3)] = 0.0f;
}

void baglGenTransform(float matrix[16], const BaglTransformConfig* config) {
  if (!matrix || !config) {
    return;
  }
  float cosX = cosf(rad(config->rotation.x));
  float cosY = cosf(rad(config->rotation.y));
  float cosZ = cosf(rad(config->rotation.z));
  float sinX = sinf(rad(config->rotation.x));
  float sinY = sinf(rad(config->rotation.y));
  float sinZ = sinf(rad(config->rotation.z));

  matrix[idx(0, 0)] = config->scale.x * (cosY * cosZ);
  matrix[idx(0, 1)] = config->scale.x * (cosY * sinZ);
  matrix[idx(0, 2)] = config->scale.x * (-sinY);
  matrix[idx(0, 3)] = 0.0f;

  matrix[idx(1, 0)] = config->scale.y * (cosZ * sinX * sinY - cosX * sinY);
  matrix[idx(1, 1)] = config->scale.y * (cosX * cosZ + sinX * sinY * sinZ);
  matrix[idx(1, 2)] = config->scale.y * (cosY * sinX);
  matrix[idx(1, 3)] = 0.0f;

  matrix[idx(2, 0)] = -config->scale.z * (sinX * sinZ + cosX * cosZ * sinY);
  matrix[idx(2, 1)] = config->scale.z * (cosX * sinY * sinZ - cosZ * sinX);
  matrix[idx(2, 2)] = config->scale.z * (cosX * cosY);
  matrix[idx(2, 3)] = 0.0f;

  matrix[idx(3, 0)] = config->position.x;
  matrix[idx(3, 1)] = config->position.y;
  matrix[idx(3, 2)] = config->position.z;
  matrix[idx(3, 3)] = 1.0f;
}

void baglGenCamView(float matrix[16], const BaglTransformConfig* config) {
  if (!matrix || !config) {
    return;
  }

  float cosX = cosf(rad(config->rotation.x));
  float cosY = cosf(rad(config->rotation.y));
  float cosZ = cosf(rad(config->rotation.z));
  float sinX = sinf(rad(config->rotation.x));
  float sinY = sinf(rad(config->rotation.y));
  float sinZ = sinf(rad(config->rotation.z));

  matrix[idx(0, 0)] = cosY * cosZ;
  matrix[idx(0, 1)] = cosY * -sinZ;
  matrix[idx(0, 2)] = sinY;
  matrix[idx(0, 3)] = 0;

  matrix[idx(1, 0)] = sinX * sinY * cosZ + cosX * sinZ;
  matrix[idx(1, 1)] = -sinX * sinY * sinZ + cosX * cosZ;
  matrix[idx(1, 2)] = -sinX * cosY;
  matrix[idx(1, 3)] = 0;

  matrix[idx(2, 0)] = cosX * sinY * cosZ - sinX * sinZ;
  matrix[idx(2, 1)] = -cosX * sinY * sinZ - sinX * cosZ;
  matrix[idx(2, 2)] = -cosX * cosY;
  matrix[idx(2, 3)] = 0;

  matrix[idx(3, 0)] =
      -dot(matrix[idx(0, 0)], matrix[idx(0, 1)], matrix[idx(0, 2)],
           config->position.x, config->position.y, config->position.z);
  matrix[idx(3, 1)] =
      -dot(matrix[idx(1, 0)], matrix[idx(1, 1)], matrix[idx(1, 2)],
           config->position.x, config->position.y, config->position.z);
  matrix[idx(3, 2)] =
      -dot(matrix[idx(2, 0)], matrix[idx(2, 1)], matrix[idx(2, 2)],
           config->position.x, config->position.y, config->position.z);
  matrix[idx(3, 3)] = 1;
}