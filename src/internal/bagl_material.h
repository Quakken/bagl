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
  } specular;
  float specularExponent;
} BaglMaterialLayout;

typedef struct BaglMaterial {
  BaglImage* diffuseMap;
  BaglImage* specularMap;
  BaglImage* normalMap;
  BaglShader* shader;
  GLuint ubo;
  bool ownsShader;
} BaglMaterial;

#endif