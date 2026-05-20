/**
 * @file material.h
 * @authors quak
 * @brief Stores material data for phong lighting.
 */

#ifndef BAGL_MATERIAL_H
#define BAGL_MATERIAL_H

typedef struct BaglState BaglState;
typedef struct BaglImage BaglImage;
typedef struct BaglShader BaglShader;

/**
 * @brief Stores configuration options for a material.
 */
typedef struct BaglMaterialConfig {
  struct {
    float r, g, b;
  } ambient;
  struct {
    float r, g, b;
  } diffuse;
  struct {
    float r, g, b;
  } specular;
  float specularExponent;

  const char** ambientFilenames;
  const char** diffuseFilenames;
  const char** specularFilenames;
  const char** normalFilenames;
  BaglImage** ambientMaps;
  BaglImage** diffuseMaps;
  BaglImage** specularMaps;
  BaglImage** normalMaps;
  size_t numAmbientMaps;
  size_t numDiffuseMaps;
  size_t numSpecularMaps;
  size_t numNormalMaps;

  BaglShader* shader;
  const char* vertexFilename;
  const char* fragmentFilename;
} BaglMaterialConfig;

/**
 * Material layout in GLSL:
 *
 * layout (std140, binding = 1) uniform Material {
 *   float specularExponent;
 *   float transparency;
 *   vec3 ambient;
 *   vec3 specular;
 * };
 */

/**
 * @brief Stores information about how an object should be colored/lit.
 */
typedef struct BaglMaterial BaglMaterial;

/**
 * @brief Creates a new material.
 * @param state State to create material with.
 * @param config Configuration to create material with.
 * @return Pointer to the material, or NULL if it could not be created.
 */
BaglMaterial* baglCreateMaterial(BaglState* state,
                                 const BaglMaterialConfig* config);

/**
 * @brief Destroys a material.
 * @param state State to destroy material with.
 * @param material Material to destroy.
 */
void baglDestroyMaterial(BaglState* state, BaglMaterial** material);

#endif