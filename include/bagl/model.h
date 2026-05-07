/**
 * @file model.h
 * @authors quak
 * @brief Stores information about a model, which can be drawn to a frame.
 */

#ifndef BAGL_MODEL_H
#define BAGL_MODEL_H

typedef struct BaglState BaglState;
typedef struct BaglMaterial BaglMaterial;
typedef struct BaglMesh BaglMesh;
typedef struct BaglFrame BaglFrame;

/**
 * @brief Stores information about a model.
 */
typedef struct BaglModel BaglModel;

/**
 * @brief Creates a new model.
 * @param state State to create model with.
 * @param mesh Mesh to create model with.
 * @param material Material to create model with. Can be NULL.
 * @return Pointer to the model, or NULL if the model could not be created.
 */
BaglModel* baglCreateModel(BaglState* state,
                           BaglMesh* mesh,
                           BaglMaterial* material);

/**
 * @brief Loads a model from an obj file.
 * @param state State to load model with.
 * @param filename Name of the file to load.
 * @return Pointer to the model, or NULL if the model could not be loaded.
 */
BaglModel* baglLoadOBJ(BaglState* state, const char* filename);

/**
 * @brief Sets the position of a model.
 * @param state State of the model to modify.
 * @param model Model to modify.
 * @param x New x position.
 * @param y New y position.
 * @param z New z position.
 */
void baglSetModelPosition(BaglState* state,
                          BaglModel* model,
                          float x,
                          float y,
                          float z);

/**
 * @brief Sets the rotation of a model.
 * @param state State of the model to modify.
 * @param model Model to modify.
 * @param x New x rotation.
 * @param y New y rotation.
 * @param z New z rotation.
 */
void baglSetModelRotation(BaglState* state,
                          BaglModel* model,
                          float x,
                          float y,
                          float z);

/**
 * @brief Sets the scale of a model.
 * @param state State of the model to modify.
 * @param model Model to modify.
 * @param x New x scale.
 * @param y New y scale.
 * @param z New z scale.
 */
void baglSetModelScale(BaglState* state,
                       BaglModel* model,
                       float x,
                       float y,
                       float z);

/**
 * @brief Destroys a model.
 * @param state State to destroy model with.
 * @param model Model to destroy.
 */
void baglDestroyModel(BaglState* state, BaglModel** model);

#endif