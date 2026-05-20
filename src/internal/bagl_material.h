/**
 * @file bagl_material.h
 * @authors quak
 * @brief Defines the BaglMaterial object.
 */

#ifndef BAGL_BAGL_MATERIAL_H
#define BAGL_BAGL_MATERIAL_H

#include <stdbool.h>

typedef unsigned int GLuint;
typedef struct BaglImage BaglImage;
typedef struct BaglShader BaglShader;

/*
 * Structure used to define the layout of a material (following std140
 * convention)
 */
typedef struct BaglMaterialLayout {
  struct {
    float r, g, b;
  } ambient;
  float padding0;
  struct {
    float r, g, b;
  } diffuse;
  float padding1;
  struct {
    float r, g, b;
  } specular;
  float specularExponent;
} BaglMaterialLayout;

typedef struct BaglMaterial {
  BaglImage** ambientMaps;
  BaglImage** diffuseMaps;
  BaglImage** specularMaps;
  BaglImage** normalMaps;
  BaglShader* shader;
  size_t numAmbientMaps;
  size_t numDiffuseMaps;
  size_t numSpecularMaps;
  size_t numNormalMaps;
  GLuint ubo;
  bool ownsAmbientMaps;
  bool ownsDiffuseMaps;
  bool ownsSpecularMaps;
  bool ownsNormalMaps;
  bool ownsShader;
} BaglMaterial;

#endif