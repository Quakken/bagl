#include "bagl/material.h"

#include <stdlib.h>
#include <string.h>

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

static bool baglLoadMaterialImages(BaglState* state,
                                   BaglImage*** images,
                                   size_t count,
                                   const char** src) {
  if (!state || !images || !src) {
    return false;
  }
  *images = state->reallocFn(NULL, count * sizeof(BaglImage*));
  if (!(*images)) {
    baglLog(state, ERROR,
            "Could not allocate images (in baglLoadMaterialImages)");
    return false;
  }
  for (size_t i = 0; i < count; ++i) {
    (*images)[i] = baglLoadImage(state, src[i]);
  }
  return true;
}

static bool baglLinkMaterialImages(BaglState* state,
                                   BaglImage*** images,
                                   size_t count,
                                   BaglImage** src) {
  if (!state || !images || !src) {
    return false;
  }
  *images = state->reallocFn(NULL, count * sizeof(BaglImage*));
  if (!(*images)) {
    baglLog(state, ERROR,
            "Could not allocate images (in baglLinkMaterialImages)");
    return false;
  }
  memcpy(*images, src, count * sizeof(BaglImage*));
  return true;
}

static bool baglLinkMaterial(BaglState* state,
                             BaglMaterial* material,
                             const BaglMaterialConfig* config) {
  if (config->ambientMaps) {
    if (!baglLinkMaterialImages(state, &material->ambientMaps,
                                config->numAmbientMaps, config->ambientMaps)) {
      return false;
    }
    material->numAmbientMaps = config->numAmbientMaps;
    material->ownsAmbientMaps = true;
  } else if (config->ambientFilenames) {
    if (!baglLoadMaterialImages(state, &material->ambientMaps,
                                config->numAmbientMaps,
                                config->ambientFilenames)) {
      return false;
    }
    material->numAmbientMaps = config->numAmbientMaps;
    material->ownsAmbientMaps = false;
  }
  if (config->diffuseMaps) {
    if (!baglLinkMaterialImages(state, &material->diffuseMaps,
                                config->numDiffuseMaps, config->diffuseMaps)) {
      return false;
    }
    material->numDiffuseMaps = config->numDiffuseMaps;
    material->ownsDiffuseMaps = true;
  } else if (config->diffuseFilenames) {
    if (!baglLoadMaterialImages(state, &material->diffuseMaps,
                                config->numDiffuseMaps,
                                config->diffuseFilenames)) {
      return false;
    }
    material->numDiffuseMaps = config->numDiffuseMaps;
    material->ownsDiffuseMaps = false;
  }
  if (config->specularMaps) {
    if (!baglLinkMaterialImages(state, &material->specularMaps,
                                config->numSpecularMaps,
                                config->specularMaps)) {
      return false;
    }
    material->numSpecularMaps = config->numSpecularMaps;
    material->ownsSpecularMaps = true;

  } else if (config->specularFilenames) {
    if (!baglLoadMaterialImages(state, &material->specularMaps,
                                config->numSpecularMaps,
                                config->specularFilenames)) {
      return false;
    }
    material->numSpecularMaps = config->numSpecularMaps;
    material->ownsSpecularMaps = false;
  }
  if (config->normalMaps) {
    if (!baglLinkMaterialImages(state, &material->normalMaps,
                                config->numNormalMaps, config->normalMaps)) {
      return false;
    }
    material->numNormalMaps = config->numNormalMaps;
    material->ownsNormalMaps = true;

  } else if (config->normalFilenames) {
    if (!baglLoadMaterialImages(state, &material->normalMaps,
                                config->numNormalMaps,
                                config->normalFilenames)) {
      return false;
    }
    material->numNormalMaps = config->numNormalMaps;
    material->ownsNormalMaps = false;
  }
  return true;
}

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
  memset(material, 0, sizeof(BaglMaterial));

  /* Link against maps */
  if (!baglLinkMaterial(state, material, config)) {
    baglLog(state, ERROR,
            "Could not link material images (in baglCreateMaterial)");
    baglDestroyMaterial(state, &material);
    return NULL;
  }

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
    if (material->diffuseMaps) {
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
  if (m->ownsAmbientMaps) {
    for (size_t i = 0; i < m->numAmbientMaps; ++i) {
      baglDestroyImage(state, &m->ambientMaps[i]);
    }
  }
  if (m->ownsDiffuseMaps) {
    for (size_t i = 0; i < m->numDiffuseMaps; ++i) {
      baglDestroyImage(state, &m->diffuseMaps[i]);
    }
  }
  if (m->ownsSpecularMaps) {
    for (size_t i = 0; i < m->numSpecularMaps; ++i) {
      baglDestroyImage(state, &m->specularMaps[i]);
    }
  }
  if (m->ownsNormalMaps) {
    for (size_t i = 0; i < m->numNormalMaps; ++i) {
      baglDestroyImage(state, &m->normalMaps[i]);
    }
  }

  if (m->ambientMaps) {
    state->reallocFn(m->ambientMaps, 0);
  }
  if (m->diffuseMaps) {
    state->reallocFn(m->diffuseMaps, 0);
  }
  if (m->specularMaps) {
    state->reallocFn(m->specularMaps, 0);
  }
  if (m->normalMaps) {
    state->reallocFn(m->normalMaps, 0);
  }
  state->reallocFn(m, 0);
  *material = NULL;
  baglLog(state, INFO, "Material destroyed");
}