/**
 * @file camera.h
 * @authors quak
 * @brief Stores information about a camera, which is required to draw objects.
 */

#ifndef BAGL_CAMERA_H
#define BAGL_CAMERA_H

typedef struct BaglState BaglState;

/**
 * @brief Stores configuration data for a camera.
 */
typedef struct BaglCameraConfig {
  float fov;
  float aspect;
  float nearPlane;
  float farPlane;
} BaglCameraConfig;

/**
 * @brief Stores information about a camera.
 */
typedef struct BaglCamera BaglCamera;

/**
 * @brief Creates a camera.
 * @param state State to create camera with.
 * @param config Configuration to create camera with.
 * @return Pointer to the camera, or NULL if the camera could not be created.
 */
BaglCamera* baglCreateCamera(BaglState* state, const BaglCameraConfig* config);

/**
 * @brief Sets the position of the camera.
 * @param state State of the camera to modify.
 * @param camera Camera to modify.
 * @param x New x position.
 * @param y New y position.
 * @param z New z position.
 */
void baglSetCameraPosition(BaglState* state,
                           BaglCamera* camera,
                           float x,
                           float y,
                           float z);

/**
 * @brief Sets the rotation of the camera.
 * @param state State of the camera to modify.
 * @param camera Camera to modify.
 * @param x New x rotation.
 * @param y New y rotation.
 * @param z New z rotation.
 */
void baglSetCameraRotation(BaglState* state,
                           BaglCamera* camera,
                           float x,
                           float y,
                           float z);

/**
 * @brief Destroys a camera.
 * @param state State of the camera to destroy.
 * @param camera Camera to destroy.
 */
void baglDestroyCamera(BaglState* state, BaglCamera** camera);

#endif