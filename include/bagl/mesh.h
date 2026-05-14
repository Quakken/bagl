/**
 * @file mesh.h
 * @authors quak
 * @brief Stores mesh data (vertices, normals, elements).
 */

#ifndef BAGL_MESH_H
#define BAGL_MESH_H

#include <stdint.h>

typedef struct BaglState BaglState;

/**
 * @brief Stores position, normal, and texture coordinate information for a
 * single vertex.
 */
typedef struct BaglVertex {
  struct {
    float x, y, z;
  } position;
  struct {
    float x, y, z;
  } normal;
  struct {
    float u, v;
  } texCoords;
} BaglVertex;

/**
 * @brief Stores information about a mesh.
 */
typedef struct BaglMesh BaglMesh;

/**
 * @brief Stores all information needed to construct a mesh.
 */
typedef struct BaglMeshConfig {
  size_t numVertices;
  BaglVertex* vertices;
  size_t numIndices;
  uint32_t* indices;
} BaglMeshConfig;

/**
 * @brief Creates a mesh.
 * @param state State to create mesh with.
 * @param config Configuration to create mesh with.
 * @return Pointer to the mesh, or NULL if the mesh could not be created.
 */
BaglMesh* baglCreateMesh(BaglState* state, const BaglMeshConfig* config);

/**
 * @brief Destroys a mesh.
 * @param state State to destroy mesh with.
 * @param mesh Mesh to destroy.
 */
void baglDestroyMesh(BaglState* state, BaglMesh** mesh);

/**
 * @brief Destroys an array of meshes.
 * @param state State to destroy mesh with.
 * @param mesh Meshes to destroy.
 * @param numMeshes Number of meshes in the array.
 */
void baglDestroyMeshes(BaglState* state, BaglMesh*** meshes, size_t numMeshes);

#endif