#include "bagl/material.h"

#include <stdlib.h>

#include "bagl/image.h"
#include "bagl/shader.h"
#include "glad/glad.h"

#include "internal/bagl_state.h"
#include "internal/bagl_material.h"
#include "internal/bagl_shader.h" /* Texture units and binding points */
#include "internal/utils.h"

/* Default material configuration used when no alternative is provided */
const static BaglMaterialConfig BAGL_DEFAULT_MATERIAL_CONFIG = {
    .ambient = {1.0f, 1.0f, 1.0f},
    .specular = {1.0f, 1.0f, 1.0f},
    .specularExponent = 200.0f,
};

BaglMaterial* baglCreateMaterial(BaglState* state,
                                 const BaglMaterialConfig* config) {
  if (!state) {
    return NULL;
  }
  if (!config) {
    config = &BAGL_DEFAULT_MATERIAL_CONFIG;
  }

  /* Allocate the material */
  BaglMaterial* material = state->reallocFn(NULL, sizeof(BaglMaterial));
  if (!material) {
    baglLog(state, ERROR,
            "Could not allocate material (in baglCreateMaterial)");
    return NULL;
  }
  material->diffuseMap = config->diffuseMap;
  material->specularMap = config->specularMap;
  material->normalMap = config->normalMap;

  /* Determine which shader to use */
  if (config->shader) {
    material->shader = config->shader;
    material->ownsShader = false;
  } else if (config->vertexFilename || config->fragmentFilename) {
    BaglShaderConfig shaderConfig = {
        .vertexFilename = config->vertexFilename,
        .fragmentFilename = config->fragmentFilename,
    };

    /* Compile the shader */
    material->shader = baglCreateShader(state, &shaderConfig);
    if (!material->shader) {
      baglLog(state, ERROR,
              "Could not compile material shader (in baglCreateMaterial)");
      state->reallocFn(material, 0);
      return NULL;
    }

    material->ownsShader = true;
  } else {
    /* Choose a default shader */
    if (material->diffuseMap) {
      material->shader = state->modelTexturedShader;
    } else {
      material->shader = state->modelColoredShader;
    }
    material->ownsShader = false;
  }

  /* Describe UBO contents */
  BaglMaterialLayout layout = {
      .ambient = {config->ambient.r, config->ambient.g, config->ambient.b},
      .specular = {config->specular.r, config->specular.g, config->specular.b},
      .specularExponent = config->specularExponent,
  };

  /* Generate the UBO */
  glGenBuffers(1, &material->ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, material->ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(BaglMaterialLayout), &layout,
               GL_STATIC_DRAW);

  baglLog(state, INFO, "Material created");
  return material;
}

void baglDestroyMaterial(BaglState* state, BaglMaterial** material) {
  if (!state || !material || !(*material)) {
    return;
  }
  BaglMaterial* m = *material;

  glDeleteBuffers(1, &m->ubo);
  if (m->ownsShader) {
    baglDestroyShader(state, &m->shader);
  }
  if (m->ownsDiffuse) {
    baglDestroyImage(state, &m->diffuseMap);
  }
  if (m->ownsSpecular) {
    baglDestroyImage(state, &m->specularMap);
  }
  if (m->ownsNormal) {
    baglDestroyImage(state, &m->normalMap);
  }

  state->reallocFn(m, 0);
  *material = NULL;
  baglLog(state, INFO, "Material destroyed");
}

void baglDestroyMaterials(BaglState* state,
                          BaglMaterial*** materials,
                          size_t numMaterials) {
  if (!state || !materials || !(*materials)) {
    return;
  }
  for (size_t i = 0; i < numMaterials; ++i) {
    BaglMaterial* material = (*materials)[i];
    baglDestroyMaterial(state, &material);
  }
  state->reallocFn(*materials, 0);
  *materials = NULL;
}