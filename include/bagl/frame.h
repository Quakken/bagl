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

typedef enum BaglAttachment {
  BAGL_ATTACHMENT_COLOR,
  BAGL_ATTACHMENT_DEPTH_STENCIL,
} BaglAttachment;

typedef struct BaglFrameConfig {
  int numColorAttachments;
  /* TODO: Allow user to pass their own images as color attachments */
  /* TODO: Allow user to pass their own depth/stencil buffers */
  bool enableDepthTest;
  bool enableStencilTest;
  /* TODO: Frame usage? (readonly, writeonly, read/write) */
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
 * @param r Red chanel value.
 * @param g Green chanel value.
 * @param b Blue chanel value.
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