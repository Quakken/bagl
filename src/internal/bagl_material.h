/**
 * @file bagl_material.h
 * @authors quak
 * @brief Defines the BaglMaterial object.
 */

#ifndef BAGL_BAGL_MATERIAL_H
#define BAGL_BAGL_MATERIAL_H

typedef unsigned int GLuint;

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
  float padding1;

  float specularExponent;
  float transparency;
  int diffuseUnit;
  int specularUnit;
  int normalUnit;
} BaglMaterialLayout;

typedef struct BaglMaterialData {
  /*
   * Diffuse, specular, and normal textures will always be bound to units 0-3
   * Only ambient/specular colors and transparency need to be updated
   */
  struct {
    float r, g, b;
    float exponent;
  } specular;
  struct {
    float r, g, b;
  } ambient;
  float transparency;
} BaglMaterialData;

typedef struct BaglMaterial {
  BaglMaterialData data;
  GLuint ubo;
} BaglMaterial;

#endif