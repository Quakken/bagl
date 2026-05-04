#include "bagl/image.h"

#include "glad/glad.h"
#include "stb/stb_image.h"

#include "internal/bagl_image.h"
#include "internal/bagl_state.h"
#include "internal/utils.h"

/* Returns the OpenGL texture format corresponding to a BaglImageFormat */
static GLenum baglGetTextureFormat(BaglImageFormat fmt);
/* Returns the OpenGL filter corresponding to a BaglFilterMode */
static GLenum baglGetFilter(BaglImageFilter mode);

BaglImage* baglCreateImage(BaglState* state, const BaglImageConfig* config) {
  /* Check parameters */
  if (!state) {
    return NULL;
  }
  if (!config) {
    baglLog(state, ERROR, "Image config cannot be NULL (in baglCreateImage)");
    return NULL;
  }
  if (config->width <= 0 || config->height <= 0) {
    baglLog(
        state, ERROR,
        "Image width and height must be greater than 0 (in baglCreateImage)");
    return NULL;
  }

  /* Allocate the image */
  BaglImage* image = state->reallocFn(NULL, sizeof(BaglImage));
  if (!image) {
    baglLog(state, ERROR, "Could not allocate image (in baglCreateImage)");
    return NULL;
  }

  /* Initialize data */
  image->width = config->width;
  image->height = config->height;
  image->format = config->format;

  /* Generate the OpenGL texture */
  GLenum textureFormat = baglGetTextureFormat(image->format);
  GLenum textureType = image->format == BAGL_FORMAT_DEPTH_STENCIL
                           ? GL_UNSIGNED_INT_24_8
                           : GL_UNSIGNED_BYTE;
  glGenTextures(1, &image->texture);
  glBindTexture(GL_TEXTURE_2D, image->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, image->width, image->height, 0,
               textureFormat, textureType, config->data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  baglGetFilter(config->minFilter));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  baglGetFilter(config->minFilter));

  baglLog(state, INFO, "Image created");
  return image;
}

BaglImage* baglLoadImage(BaglState* state, const char* filename) {
  if (!state || !filename) {
    return NULL;
  }

  /* Load image data */
  int width;
  int height;
  stbi_set_flip_vertically_on_load(true);
  uint8_t* data = stbi_load(filename, &width, &height, NULL, 4);
  if (!data) {
    baglLog(state, ERROR, "Could not load image data (in baglLoadImage)");
    return NULL;
  }

  /* Create the image */
  BaglImageConfig config = {
      .width = width,
      .height = height,
      .format = BAGL_FORMAT_RGBA,
      .data = data,
  };
  BaglImage* image = baglCreateImage(state, &config);
  stbi_image_free(data);
  return image;
}

void baglDestroyImage(BaglState* state, BaglImage** image) {
  if (!state || !image || !(*image)) {
    return;
  }
  BaglImage* i = *image;

  /* Destroy the texture */
  glDeleteTextures(1, &i->texture);

  /* Release memory */
  state->reallocFn(i, 0);
  *image = NULL;

  baglLog(state, INFO, "Image destroyed");
}

static GLenum baglGetTextureFormat(BaglImageFormat fmt) {
  switch (fmt) {
    case BAGL_FORMAT_RGBA:
      return GL_RGBA;
    case BAGL_FORMAT_RGB:
      return GL_RGB;
    case BAGL_FORMAT_DEPTH_STENCIL:
      return GL_DEPTH_STENCIL;
  }
}

static GLenum baglGetFilter(BaglImageFilter mode) {
  switch (mode) {
    case BAGL_FILTER_LINEAR:
      return GL_LINEAR;
    case BAGL_FILTER_NEAREST:
      return GL_NEAREST;
  }
}