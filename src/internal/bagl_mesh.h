/**
 * @file bagl_mesh.h
 * @authors quak
 * @brief Defines the BaglMesh object.
 */

#ifndef BAGL_BAGL_MESH_H
#define BAGL_BAGL_MESH_H

typedef unsigned int GLuint;

typedef struct BaglMesh {
  GLuint vbo;
  GLuint ebo;
  GLuint vao;
} BaglMesh;

#endif