/**
 * @file image.h
 * @authors quak
 * @brief Stores information about an image.
 */

#ifndef BAGL_IMAGE_H
#define BAGL_IMAGE_H

#include <stdint.h> /* uint8_t */

/* Forward */

typedef struct BaglState BaglState;

/**
 * @brief Describes the format of an image's pixel data.
 */
typedef enum BaglImageFormat {
  /* 8 bit RGBA channels */
  BAGL_FORMAT_RGBA,
  /* 8 bit RGB channels */
  BAGL_FORMAT_RGB,
  /* 24 bit depth channel, 8 bit stencil channel */
  BAGL_FORMAT_DEPTH_STENCIL,
  /* Unknown format */
  BAGL_FORMAT_UNKNOWN,
} BaglImageFormat;

/**
 * @brief Describes a filter used to scale an image.
 */
typedef enum BaglImageFilter {
  BAGL_FILTER_LINEAR,
  BAGL_FILTER_NEAREST,
} BaglImageFilter;

/**
 * @brief Stores configuration options for a BaglImage.
 */
typedef struct BaglImageConfig {
  /* Width of the image */
  int width;
  /* Height of the image */
  int height;
  /* Format of the image's pixel data */
  BaglImageFormat format;
  /* Filter to apply when shrinking image */
  BaglImageFilter minFilter;
  /* Filter to apply when magnifying image */
  BaglImageFilter magFilter;
  /* Data to load into image */
  uint8_t* data;
} BaglImageConfig;

/**
 * @brief Stores information about an image
 */
typedef struct BaglImage BaglImage;

/**
 * @brief Creates an image.
 * @param state State to create image with.
 * @param config Configuration to use when creating the image.
 * @return Pointer to the image, or NULL if the image could not be created.
 */
BaglImage* baglCreateImage(BaglState* state, const BaglImageConfig* config);

/**
 * @brief Loads an image from file.
 * @param state State to load image with.
 * @param filename Name of the image to load.
 * @return Pointer to the loaded image, or NULL if the image could not be
 * loaded.
 * @note Images will always be loaded in RGBA format. This that means they
 * cannot be used as depth/stencil attachments.
 */
BaglImage* baglLoadImage(BaglState* state, const char* filename);

/**
 * @brief Returns the width of an image.
 * @param image Image to test.
 * @return Width of the image.
 */
int baglGetImageWidth(BaglImage* image);

/**
 * @brief Returns the height of an image.
 * @param image Image to test.
 * @return Height of the image.
 */
int baglGetImageHeight(BaglImage* image);

/**
 * @brief Returns the format of an image.
 * @param image Image to test.
 * @return Format of the image.
 */
BaglImageFormat baglGetImageFormat(BaglImage* image);

/**
 * @brief Destroys an image.
 * @param state State to destroy image with.
 * @param image Image to destroy.
 */
void baglDestroyImage(BaglState* state, BaglImage** image);

#endif