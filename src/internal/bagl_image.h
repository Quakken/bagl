/**
 * @file bagl_image.h
 * @authors quak
 * @brief Defines the bagl image object.
 */

#ifndef BAGL_BAGL_IMAGE_H
#define BAGL_BAGL_IMAGE_H

#include "bagl/image.h"

typedef unsigned int GLuint;

typedef struct BaglImage {
  GLuint texture;
  int width;
  int height;
  BaglImageFormat format;
} BaglImage;

#endif