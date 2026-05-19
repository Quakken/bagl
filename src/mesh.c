#include "bagl/mesh.h"

#include <stddef.h>

#include "glad/glad.h"

#include "internal/bagl_state.h"
#include "internal/bagl_mesh.h"
#include "internal/utils.h"

/* Validates a mesh configuration */
static bool baglValidateMeshConfig(BaglState* state,
                                   const BaglMeshConfig* config);

BaglMesh* baglCreateMesh(BaglState* state, const BaglMeshConfig* config) {
  if (!state) {
    return NULL;
  }
  /* Validate the configuration */
  if (!baglValidateMeshConfig(state, config)) {
    return NULL;
  }

  /* Allocate the mesh*/
  BaglMesh* mesh = state->reallocFn(NULL, sizeof(BaglMesh));
  if (!mesh) {
    baglLog(state, ERROR, "Couldn't allocate mesh (in baglCreateMesh)");
    return NULL;
  }
  mesh->numIndices = config->numIndices;
  mesh->material = NULL;

  /* Make the VAO */
  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  /* Make the VBO and write contents */
  glGenBuffers(1, &mesh->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, config->numVertices * sizeof(BaglVertex),
               config->vertices, GL_STATIC_DRAW);

  /* Make the EBO and write contents */
  glGenBuffers(1, &mesh->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, config->numIndices * sizeof(uint32_t),
               config->indices, GL_STATIC_DRAW);

  /* Write VAO attributes */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BaglVertex),
                        (void*)offsetof(BaglVertex, position));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BaglVertex),
                        (void*)offsetof(BaglVertex, normal));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BaglVertex),
                        (void*)offsetof(BaglVertex, texCoords));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  baglLog(state, INFO, "Mesh created");
  return mesh;
}

void baglDestroyMesh(BaglState* state, BaglMesh** mesh) {
  if (!state || !mesh || !(*mesh)) {
    return;
  }
  BaglMesh* m = *mesh;

  glDeleteBuffers(1, &m->vbo);
  glDeleteBuffers(1, &m->ebo);
  glDeleteVertexArrays(1, &m->vao);

  state->reallocFn(m, 0);
  *mesh = NULL;
  baglLog(state, INFO, "Mesh destroyed");
}

static bool baglValidateMeshConfig(BaglState* state,
                                   const BaglMeshConfig* config) {
  if (!config) {
    baglLog(state, ERROR, "Config cannot be null (in baglCreateMesh)");
    return false;
  }
  if (config->numVertices <= 0) {
    baglLog(state, ERROR,
            "Number of vertices must be greater than 0 (in baglCreateMesh)");
    return false;
  }
  if (!config->vertices) {
    baglLog(state, ERROR, "No vertices supplied (in baglCreateMesh)");
    return false;
  }
  if (config->numIndices <= 0) {
    baglLog(state, ERROR,
            "Number of indices must be greater than 0 (in baglCreateMesh)");
    return false;
  }
  if (!config->indices) {
    baglLog(state, ERROR, "No indices supplied (in baglCreateMesh)");
    return false;
  }
  return true;
}