/**
 * @file bagl_mesh.h
 * @authors quak
 * @brief Defines the BaglMesh object.
 */

#ifndef BAGL_BAGL_MESH_H
#define BAGL_BAGL_MESH_H

typedef struct BaglMaterial BaglMaterial;
typedef unsigned int GLuint;

typedef struct BaglMesh {
  GLuint vbo;
  GLuint ebo;
  GLuint vao;
  unsigned int numIndices;
  BaglMaterial* material; /* used by BaglModel */
} BaglMesh;

#endif