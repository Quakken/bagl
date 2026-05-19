#include "bagl/model.h"

#include <stdbool.h>
#include <string.h>

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include "assimp/importerdesc.h"
#include "assimp/scene.h"
#include "bagl/mesh.h"
#include "bagl/material.h"

#include "internal/bagl_model.h"
#include "internal/bagl_state.h"
#include "internal/bagl_mesh.h"
#include "internal/bagl_material.h"
#include "internal/utils.h"

BaglModel* baglCreateModel(BaglState* state,
                           BaglMesh* mesh,
                           BaglMaterial* material) {
  if (!state) {
    return NULL;
  }
  if (!mesh) {
    baglLog(state, ERROR, "Mesh cannot be NULL (in baglCreateModel)");
    return NULL;
  }

  /* Allocate the model */
  BaglModel* model = state->reallocFn(NULL, sizeof(BaglModel));
  if (!model) {
    baglLog(state, ERROR, "Could not allocate model (in baglCreateModel)");
    return NULL;
  }
  /* Allocate the mesh/materials */
  model->meshes = state->reallocFn(NULL, sizeof(BaglMesh*));
  if (!model->meshes) {
    baglLog(state, ERROR, "Could not allocate meshes (in baglCreateModel)");
    state->reallocFn(model, 0);
    return NULL;
  }
  model->materials = state->reallocFn(NULL, sizeof(BaglMaterial*));
  if (!model->materials) {
    baglLog(state, ERROR, "Could not allocate materials (in baglCreateModel)");
    state->reallocFn(model->meshes, 0);
    state->reallocFn(model, 0);
    return NULL;
  }

  /* Initialize members */
  model->meshes[0] = mesh;
  model->ownsMeshes = false;
  if (material) {
    model->materials[0] = material;
    model->ownsMaterials = false;
  } else {
    model->materials[0] = baglCreateMaterial(state, NULL);
    model->ownsMaterials = true;
  }
  model->numMeshes = 1;
  model->capMeshes = 1;
  model->numMaterials = 1;
  memset(&model->transformConfig, 0, sizeof(model->transformConfig));
  model->isTransformDirty = true;

  model->meshes[0]->material = model->materials[0];

  baglLog(state, INFO, "Model created");
  return model;
}

bool baglPushMesh(BaglState* state, BaglModel* model, BaglMesh* mesh) {
  if (!state || !model || !mesh) {
    return false;
  }

  /* Reallocate array if necessary */
  if (model->numMeshes >= model->capMeshes) {
    model->capMeshes = (model->capMeshes) ? model->capMeshes * 2 : 1;
    void* result =
        state->reallocFn(model->meshes, model->capMeshes * sizeof(BaglMesh*));
    if (!result) {
      baglLog(state, ERROR,
              "Could not reallocate mesh array (in baglPushMesh)");
      return false;
    }
    model->meshes = result;
  }

  model->meshes[model->numMeshes] = mesh;
  ++model->numMeshes;
  return true;
}

bool baglLoadModelMesh(BaglState* state,
                       BaglModel* model,
                       const struct aiScene* scene,
                       const struct aiMesh* mesh) {
  if (!state || !model || !scene || !mesh) {
    return false;
  }

  /* Determine how many indices need to be stored */
  size_t numIndices = 0;
  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    numIndices += mesh->mFaces[i].mNumIndices;
  }
  /* Allocate indices */
  unsigned int* indices =
      state->reallocFn(NULL, sizeof(unsigned int) * numIndices);
  if (!indices) {
    baglLog(state, ERROR, "Could not allocate indices (in baglLoadModelMesh)");
    return false;
  }

  /* Allocate vertices */
  size_t numVertices = mesh->mNumVertices;
  BaglVertex* vertices =
      state->reallocFn(NULL, sizeof(BaglVertex) * numVertices);
  if (!vertices) {
    baglLog(state, ERROR, "Could not allocate vertices (in baglLoadModelMesh)");
    state->reallocFn(indices, 0);
    return false;
  }

  /* Process all vertex data */
  for (size_t i = 0; i < mesh->mNumVertices; ++i) {
    BaglVertex vtx = {};
    vtx.position.x = mesh->mVertices[i].x;
    vtx.position.y = mesh->mVertices[i].y;
    vtx.position.z = mesh->mVertices[i].z;
    if (mesh->mNormals) {
      vtx.normal.x = mesh->mNormals[i].x;
      vtx.normal.y = mesh->mNormals[i].y;
      vtx.normal.z = mesh->mNormals[i].z;
    }
    if (mesh->mTextureCoords[0]) {
      vtx.texCoords.u = mesh->mTextureCoords[0][i].x;
      vtx.texCoords.v = mesh->mTextureCoords[0][i].x;
    }
    vertices[i] = vtx;
  }

  /* Process all indices */
  size_t currentIdx = 0;
  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    const struct aiFace face = mesh->mFaces[i];
    for (size_t j = 0; j < face.mNumIndices; ++j) {
      indices[currentIdx] = face.mIndices[j];
      ++currentIdx;
    }
  }

  /* Create the mesh */
  BaglMeshConfig meshConfig = {
      .indices = indices,
      .numIndices = numIndices,
      .vertices = vertices,
      .numVertices = numVertices,
  };
  BaglMesh* baglMesh = baglCreateMesh(state, &meshConfig);
  state->reallocFn(indices, 0);
  state->reallocFn(vertices, 0);
  if (!baglMesh) {
    state->reallocFn(indices, 0);
    state->reallocFn(vertices, 0);
    baglLog(state, ERROR, "Could not create mesh (in baglLoadModelMesh)");
    return false;
  }

  /* Link material */
  /* TODO */
  baglMesh->material = model->materials[0];

  baglPushMesh(state, model, baglMesh);

  return true;
}

bool baglLoadModelNode(BaglState* state,
                       BaglModel* model,
                       const struct aiScene* scene,
                       struct aiNode* node) {
  if (!state || !model || !scene || !node) {
    return false;
  }

  /* Process meshes */
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    if (!baglLoadModelMesh(state, model, scene, mesh)) {
      return false;
    }
  }
  /* Process children */
  for (size_t i = 0; i < node->mNumChildren; ++i) {
    if (!baglLoadModelNode(state, model, scene, node->mChildren[i])) {
      return false;
    }
  }

  return true;
}

BaglModel* baglLoadModel(BaglState* state, const char* filename) {
  if (!state || !filename) {
    return NULL;
  }

  /* Load the scene from file */
  enum aiImporterFlags flags = aiProcess_Triangulate | aiProcess_FlipUVs;
  const struct aiScene* scene = aiImportFile(filename, flags);
  if (scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    baglLog(state, ERROR, aiGetErrorString());
    baglLog(state, ERROR, "Could not load model (in baglLoadModel)");
    return NULL;
  }

  /* Allocate the model */
  BaglModel* model = state->reallocFn(NULL, sizeof(BaglModel));
  if (!model) {
    baglLog(state, ERROR, "Could not allocate model (in baglLoadModel)");
    aiReleaseImport(scene);
    return NULL;
  }
  /* Allocate materials */
  model->materials =
      state->reallocFn(NULL, sizeof(BaglMaterial*) * scene->mNumMaterials);
  if (!model->materials) {
    baglLog(state, ERROR, "Could not allocate materials (in baglLoadModel)");
    state->reallocFn(model, 0);
    aiReleaseImport(scene);
  }
  /* Allocate meshes */
  model->meshes = state->reallocFn(NULL, sizeof(BaglModel*));
  if (!model->meshes) {
    baglLog(state, ERROR, "Could not allocate meshes (in baglLoadModel)");
    state->reallocFn(model->materials, 0);
    state->reallocFn(model, 0);
    aiReleaseImport(scene);
  }
  model->numMeshes = 0;
  model->capMeshes = 1;

  model->ownsMeshes = true;
  model->ownsMaterials = true;
  memset(&model->transformConfig, 0, sizeof(model->transformConfig));
  model->isTransformDirty = true;

  /* Load materials */
  /* TODO */
  model->materials[0] = baglCreateMaterial(state, NULL);
  model->numMaterials = 1;

  /* Process each node of the scene */
  baglLoadModelNode(state, model, scene, scene->mRootNode);

  aiReleaseImport(scene);
  return model;
}

void baglSetModelPosition(BaglState* state,
                          BaglModel* model,
                          float x,
                          float y,
                          float z) {
  if (!state || !model) {
    return;
  }
  model->transformConfig.position.x = x;
  model->transformConfig.position.y = y;
  model->transformConfig.position.z = z;
  model->isTransformDirty = true;
}

void baglSetModelRotation(BaglState* state,
                          BaglModel* model,
                          float x,
                          float y,
                          float z) {
  if (!state || !model) {
    return;
  }
  model->transformConfig.rotation.x = x;
  model->transformConfig.rotation.y = y;
  model->transformConfig.rotation.z = z;
  model->isTransformDirty = true;
}

void baglSetModelScale(BaglState* state,
                       BaglModel* model,
                       float x,
                       float y,
                       float z) {
  if (!state || !model) {
    return;
  }
  model->transformConfig.scale.x = x;
  model->transformConfig.scale.y = y;
  model->transformConfig.scale.z = z;
  model->isTransformDirty = true;
}

void baglDestroyModel(BaglState* state, BaglModel** model) {
  if (!state || !model || !(*model)) {
    return;
  }
  BaglModel* m = *model;

  if (m->ownsMeshes) {
    for (size_t i = 0; i < m->numMeshes; ++i) {
      BaglMesh* mesh = m->meshes[i];
      baglDestroyMesh(state, &mesh);
    }
  }
  if (m->ownsMaterials) {
    for (size_t i = 0; i < m->numMaterials; ++i) {
      BaglMaterial* material = m->materials[i];
      baglDestroyMaterial(state, &material);
    }
  }

  state->reallocFn(m->meshes, 0);
  state->reallocFn(m->materials, 0);
  state->reallocFn(m, 0);
  *model = NULL;
  baglLog(state, INFO, "Model destroyed");
}