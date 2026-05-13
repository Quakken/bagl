/**
 * @file bagl_obj.h
 * @authors quak
 * @brief Functions to load meshes/materials from obj/mtl files.
 */

#ifndef BAGL_OBJ_H
#define BAGL_OBJ_H

typedef struct BaglState BaglState;
typedef struct BaglMesh BaglMesh;
typedef struct BaglMaterial BaglMaterial;
typedef struct BaglModel BaglModel;

/**
 * @brief Loads a mesh from an obj file.
 * @param state State to load mesh with.
 * @param filename Name of the file to load.
 * @return Pointer to the mesh, or NULL if the mesh could not be loaded.
 */
BaglMesh* baglLoadOBJ(BaglState* state, const char* filename);

/**
 * @brief Loads materials from an mtl file.
 * @param state State to load materials with.
 * @param materials Pointer to the array of materials where output should be
 * stored.
 * @param filename Name of the file to load.
 * @return Number of materials that were loaded.
 */
size_t baglLoadMTL(BaglState* state,
                   BaglMaterial*** materials,
                   const char* filename);

/**
 * @brief Loads a model from an obj/mtl file pair.
 * @param state State to load the model with.
 * @param objFilename Name of the obj file to load.
 * @param mtlName Name of the MTL file to load.
 * @return Pointer to the model, or NULL if the model could not be loaded.
 */
BaglModel* baglLoadModel(BaglState* state,
                         const char* objFilename,
                         const char* mtlFilename);

#endif