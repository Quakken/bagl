/**
 * @file bagl_material.h
 * @authors quak
 * @brief Defines the BaglMaterial object.
 */

#ifndef BAGL_BAGL_MATERIAL_H
#define BAGL_BAGL_MATERIAL_H

typedef unsigned int GLuint;
typedef struct BaglImage BaglImage;

/*
 * Structure used to define the layout of a material (following std140
 * convention)
 */
typedef struct BaglMaterialLayout {
  float specularExponent;
  float transparency;
  float padding0[2];
  struct {
    float r, g, b;
  } ambient;
  float padding1;
  struct {
    float r, g, b;
  } specular;
} BaglMaterialLayout;

typedef struct BaglMaterial {
  BaglImage* diffuseMap;
  BaglImage* specularMap;
  BaglImage* normalMap;
  GLuint ubo;
} BaglMaterial;

#endif