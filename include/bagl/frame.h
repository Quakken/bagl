/**
 * @file frame.h
 * @authors quak
 * @brief Stores information about rendering targets. Frames are required to
 * perform any rendering operation.
 */

#ifndef BAGL_FRAME_H
#define BAGL_FRAME_H

#include <stdbool.h>

/* Forward */

typedef struct BaglState BaglState;
typedef struct BaglImage BaglImage;

/**
 * @brief Describes a type of frame attachment.
 */
typedef enum BaglAttachment {
  BAGL_ATTACHMENT_COLOR,
  BAGL_ATTACHMENT_DEPTH_STENCIL,
} BaglAttachment;

/**
 * @brief Configuration options for creating a frame.
 */
typedef struct BaglFrameConfig {
  /* Number of color attachments to bind */
  size_t numColorAttachments;
  /*
   * Array of color attachments to bind to the frame. If NULL, and
   * numColorAttachments is greater than 0, the frame will generate its own
   * color attachments.
   */
  BaglImage** colorAttachments;
  /*
   * Pointer to the frame's depth/stencil attachment. If NULL, and depth or
   * stencil testing is enabled, a new attachment will be generated.
   */
  BaglImage* depthStencilAttachment;
  /* Whether to enable depth testing when writing to the frame */
  bool enableDepthTest;
  /* Whether to enable stencil testing when writing to the frame */
  bool enableStencilTest;

  /* TODO: Frame usage? (readonly, writeonly, read/write) - could be used for
   * optimization (render buffers instead of textures for color attachment) */
} BaglFrameConfig;

/**
 * @brief Stores information about rendering attachments. Required to perform
 * render operations.
 */
typedef struct BaglFrame BaglFrame;

/**
 * @brief Creates a frame.
 * @param state Bagl state to use.
 * @param config Configuration to create frame with.
 */
BaglFrame* baglCreateFrame(BaglState* state, const BaglFrameConfig* config);

/**
 * @brief Clears an attachment bound to a frame.
 * @param frame Frame to modify.
 * @param attachment Attachment to clear.
 * @param r Red channel value.
 * @param g Green channel value.
 * @param b Blue channel value.
 * @param a Alpha value.
 */
void baglClearFrame(BaglFrame* frame,
                    BaglAttachment attachment,
                    float r,
                    float g,
                    float b,
                    float a);

/**
 * @brief Presents a frame to the screen.
 * @param state Bagl state to present to.
 * @param frame Frame to present.
 * TODO: @param shader Shader to apply when presenting.
 */
void baglPresent(BaglState* state, const BaglFrame* frame, void* shader);

/**
 * @brief Destroys a frame.
 * @param state Bagl state that the frame belongs to.
 * @param frame Frame to destroy.
 */
void baglDestroyFrame(BaglState* state, BaglFrame** frame);

/* TODO: Getting attachments from frame */

#endif