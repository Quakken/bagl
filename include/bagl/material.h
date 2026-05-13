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
  } specular;
  float specularExponent;
  BaglImage* diffuseMap;
  BaglImage* specularMap;
  BaglImage* normalMap;

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

/**
 * @brief Destroys an array of materials.
 * @param state State to destroy materials with.
 * @param materials Materials to destroy.
 * @param numMaterials Number of materials in the array.
 */
void baglDestroyMaterials(BaglState* state,
                          BaglMaterial*** materials,
                          size_t numMaterials);

#endif