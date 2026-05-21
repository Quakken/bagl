#include "bagl/model.h"

#include <stdbool.h>
#include <string.h>

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include "assimp/importerdesc.h"
#include "assimp/material.h"
#include "assimp/scene.h"
#include "bagl/image.h"
#include "bagl/mesh.h"
#include "bagl/material.h"

#include "internal/bagl_model.h"
#include "internal/bagl_state.h"
#include "internal/bagl_mesh.h"
#include "internal/bagl_material.h"
#include "internal/utils.h"

const static size_t BAGL_MAX_TEX_PATH_LEN = 128;

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

static bool baglPushMesh(BaglState* state, BaglModel* model, BaglMesh* mesh) {
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

static bool baglLoadModelMesh(BaglState* state,
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
      vtx.texCoords.v = mesh->mTextureCoords[0][i].y;
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
  if (mesh->mMaterialIndex >= 0) {
    baglMesh->material = model->materials[mesh->mMaterialIndex];
  } else {
    baglMesh->material = NULL;
  }

  baglPushMesh(state, model, baglMesh);

  return true;
}

static bool baglLoadModelNode(BaglState* state,
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

static size_t baglGetLastDirOffs(const char* path) {
  size_t len = strlen(path);
  for (size_t i = len - 1; i > 0; --i) {
    if (path[i] == '/' || path[i] == '\\') {
      return i;
    }
  }
  return len;
}

static void baglResetPathRoot(char* root, size_t lastDirOffs) {
  if (!root) {
    return;
  }
  memset(root + lastDirOffs + 1, 0, BAGL_MAX_TEX_PATH_LEN - 1);
}

static char* baglGetPathRoot(BaglState* state,
                             const char* path,
                             size_t lastDirOffs) {
  if (!state || !path) {
    return NULL;
  }
  /* Use directory offset to get just the "root" part of a file */
  const size_t bufferLen = lastDirOffs + BAGL_MAX_TEX_PATH_LEN;
  char* modelRoot = state->reallocFn(NULL, bufferLen);
  if (!modelRoot) {
    baglLog(state, ERROR, "Could not allocate path (in baglGetPathRoot)");
    return NULL;
  }
  /* Copy root directory to the path */
  memcpy(modelRoot, path, lastDirOffs + 1);
  /* Zero out the rest of the path */
  baglResetPathRoot(modelRoot, lastDirOffs);
  return modelRoot;
}

static void baglAppendPathRoot(char* root,
                               const char* path,
                               size_t lastDirOffs) {
  if (!root) {
    return;
  }
  strcat_s(root, lastDirOffs + BAGL_MAX_TEX_PATH_LEN, path);
}

static void baglLoadMaterials(BaglState* state,
                              BaglModel* model,
                              const char* filename,
                              const struct aiScene* scene) {
  /* Calculate the model's directory */
  size_t lastDirOffs = baglGetLastDirOffs(filename);
  char* root = baglGetPathRoot(state, filename, lastDirOffs);

  /* Load materials */
  for (size_t i = 0; i < model->numMaterials; ++i) {
    struct aiMaterial* material = scene->mMaterials[i];
    size_t numAmbient =
        aiGetMaterialTextureCount(material, aiTextureType_AMBIENT);
    size_t numDiffuse =
        aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE);
    size_t numSpec =
        aiGetMaterialTextureCount(material, aiTextureType_SPECULAR);
    size_t numNormal =
        aiGetMaterialTextureCount(material, aiTextureType_NORMALS);

    BaglMaterialConfig config = {};
    config.ambientMaps =
        (numAmbient) ? state->reallocFn(NULL, numAmbient * sizeof(BaglImage*))
                     : NULL;
    config.diffuseMaps =
        (numDiffuse) ? state->reallocFn(NULL, numDiffuse * sizeof(BaglImage*))
                     : NULL;
    config.specularMaps =
        (numSpec) ? state->reallocFn(NULL, numSpec * sizeof(BaglImage*)) : NULL;
    config.normalMaps =
        (numNormal) ? state->reallocFn(NULL, numNormal * sizeof(BaglImage*))
                    : NULL;
    // TODO: null check
    config.numAmbientMaps = numAmbient;
    config.numDiffuseMaps = numDiffuse;
    config.numSpecularMaps = numSpec;
    config.numNormalMaps = numNormal;

    for (size_t j = 0; j < numAmbient; ++j) {
      struct aiString ambientPath = {};
      aiGetMaterialTexture(material, aiTextureType_DIFFUSE, j, &ambientPath,
                           NULL, NULL, NULL, NULL, NULL, NULL);
      baglAppendPathRoot(root, ambientPath.data, lastDirOffs);
      config.ambientMaps[j] = baglLoadImage(state, root);
      baglResetPathRoot(root, lastDirOffs);
    }
    for (size_t j = 0; j < numDiffuse; ++j) {
      struct aiString diffusePath = {};
      aiGetMaterialTexture(material, aiTextureType_DIFFUSE, j, &diffusePath,
                           NULL, NULL, NULL, NULL, NULL, NULL);
      baglAppendPathRoot(root, diffusePath.data, lastDirOffs);
      config.diffuseMaps[j] = baglLoadImage(state, root);
      baglResetPathRoot(root, lastDirOffs);
    }
    for (size_t j = 0; j < numSpec; ++j) {
      struct aiString specPath = {};
      aiGetMaterialTexture(material, aiTextureType_SPECULAR, j, &specPath, NULL,
                           NULL, NULL, NULL, NULL, NULL);
      baglAppendPathRoot(root, specPath.data, lastDirOffs);
      config.specularMaps[j] = baglLoadImage(state, root);
      baglResetPathRoot(root, lastDirOffs);
    }
    for (size_t j = 0; j < numNormal; ++j) {
      struct aiString normPath = {};
      aiGetMaterialTexture(material, aiTextureType_NORMALS, j, &normPath, NULL,
                           NULL, NULL, NULL, NULL, NULL);
      baglAppendPathRoot(root, normPath.data, lastDirOffs);
      config.normalMaps[j] = baglLoadImage(state, root);
      baglResetPathRoot(root, lastDirOffs);
    }

    struct aiColor4D ambientColor = {};
    struct aiColor4D diffuseColor = {};
    struct aiColor4D specColor = {};
    aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor);
    aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
    aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specColor);
    aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &config.specularExponent);
    config.ambient.r = ambientColor.r;
    config.ambient.g = ambientColor.g;
    config.ambient.b = ambientColor.b;
    config.diffuse.r = diffuseColor.r;
    config.diffuse.g = diffuseColor.g;
    config.diffuse.b = diffuseColor.b;
    config.specular.r = specColor.r;
    config.specular.g = specColor.g;
    config.specular.b = specColor.b;

    model->materials[i] = baglCreateMaterial(state, &config);
    /* Free configuration and transfer ownership to material */
    if (model->materials[i]) {
      model->materials[i]->ownsAmbientMaps = true;
      model->materials[i]->ownsDiffuseMaps = true;
      model->materials[i]->ownsSpecularMaps = true;
      model->materials[i]->ownsNormalMaps = true;
    } else {
      for (size_t i = 0; i < config.numAmbientMaps; ++i) {
        baglDestroyImage(state, &config.ambientMaps[i]);
      }
      for (size_t i = 0; i < config.numDiffuseMaps; ++i) {
        baglDestroyImage(state, &config.diffuseMaps[i]);
      }
      for (size_t i = 0; i < config.numSpecularMaps; ++i) {
        baglDestroyImage(state, &config.specularMaps[i]);
      }
      for (size_t i = 0; i < config.numNormalMaps; ++i) {
        baglDestroyImage(state, &config.normalMaps[i]);
      }
    }
    if (config.ambientMaps) {
      state->reallocFn(config.ambientMaps, 0);
    }
    if (config.diffuseMaps) {
      state->reallocFn(config.diffuseMaps, 0);
    }
    if (config.specularMaps) {
      state->reallocFn(config.specularMaps, 0);
    }
    if (config.normalMaps) {
      state->reallocFn(config.normalMaps, 0);
    }
  }

  /* Release path root */
  state->reallocFn(root, 0);
}

BaglModel* baglLoadModel(BaglState* state, const char* filename) {
  if (!state || !filename) {
    return NULL;
  }

  /* Load the scene from file */
  const struct aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);
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
    return NULL;
  }
  model->numMaterials = scene->mNumMaterials;

  /* Allocate meshes */
  model->meshes = state->reallocFn(NULL, sizeof(BaglModel*));
  if (!model->meshes) {
    baglLog(state, ERROR, "Could not allocate meshes (in baglLoadModel)");
    state->reallocFn(model->materials, 0);
    state->reallocFn(model, 0);
    aiReleaseImport(scene);
    return NULL;
  }
  model->numMeshes = 0;
  model->capMeshes = 1;

  /* Assign members */
  model->ownsMeshes = true;
  model->ownsMaterials = true;
  memset(&model->transformConfig, 0, sizeof(model->transformConfig));
  model->isTransformDirty = true;

  /* Load material data */
  baglLoadMaterials(state, model, filename, scene);

  /* Process each node of the scene */
  bool result = baglLoadModelNode(state, model, scene, scene->mRootNode);
  if (!result) {
    baglLog(state, ERROR,
            "Error occurred when loading model (in baglLoadModel)");
  }

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