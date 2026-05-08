/**
 * @file frame.h
 * @authors quak
 * @brief Stores information about rendering targets. Frames are required to
 * perform any rendering operation.
 */

#ifndef BAGL_FRAME_H
#define BAGL_FRAME_H

#include <stdbool.h> /* bool */

/* Forward */

typedef struct BaglState BaglState;
typedef struct BaglImage BaglImage;
typedef struct BaglShader BaglShader;
typedef struct BaglModel BaglModel;
typedef struct BaglCamera BaglCamera;

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
 * @param state State to use.
 * @param frame Frame to modify.
 * @param attachment Attachment to clear.
 * @param r Red channel value.
 * @param g Green channel value.
 * @param b Blue channel value.
 * @param a Alpha value.
 */
void baglClearFrame(BaglState* state,
                    BaglFrame* frame,
                    BaglAttachment attachment,
                    float r,
                    float g,
                    float b,
                    float a);

/**
 * @brief Returns all color attachments bound to a frame.
 * @param state State to use
 * @param frame Frame to get color attachments from.
 * @param count Output parameter to store number of color attachments returned.
 * @return Pointer to the array of color attachments bound to the frame.
 */
BaglImage** baglGetColorAttachments(BaglState* sate,
                                    const BaglFrame* frame,
                                    size_t* count);

/**
 * @brief Returns the depth/stencil attachment bound to the frame.
 * @param state State to use.
 * @brief frame Frame to get depth/stencil attachment from.
 * @return Pointer to the frame's depth/stencil attachment.
 */
BaglImage* baglGetDepthStencilAttachment(BaglState* state,
                                         const BaglFrame* frame);

/**
 * @brief Enables/disables depth testing for the frame.
 * @param state State to use.
 * @param frame Frame to modify.
 * @param enabled Whether depth tests should be enabled/disabled.
 */
void baglSetDepthTestEnabled(BaglState* state, BaglFrame* frame, bool enabled);

/**
 * @brief Enables/disables stencil testing for the frame.
 * @param state State to use.
 * @param frame Frame to modify.
 * @param enabled Whether stencil tests should be enabled/disabled.
 */
void baglSetStencilTestEnabled(BaglState* state,
                               BaglFrame* frame,
                               bool enabled);

/**
 * @brief Applies a post-processing effect to a frame.
 * @param state State to use.
 * @param dest Frame to write to.
 * @param src Frame to read from.
 * @param shader Shader to apply. This should be a fullscreen shader (created
 * without specifying a vertex stage).
 */
void baglProcessFrame(BaglState* state,
                      BaglFrame* dest,
                      const BaglFrame* src,
                      BaglShader* shader);

/**
 * @brief Draws a model to a frame.
 * @param state State to draw with.
 * @param frame Frame to draw to.
 * @param model Model to draw.
 * @param shader Shader to draw model with.
 * @param camera Camera to draw model with.
 */
void baglDraw(BaglState* state,
              BaglFrame* frame,
              BaglModel* model,
              BaglShader* shader,
              BaglCamera* camera);

/**
 * @brief Presents a frame to the screen.
 * @param state Bagl state to present to.
 * @param frame Frame to present.
 * @param shader Shader to apply when presenting. This should be a fullscreen
 * shader (created without specifying a vertex stage).
 */
void baglPresent(BaglState* state, const BaglFrame* frame, BaglShader* shader);

/**
 * @brief Destroys a frame.
 * @param state Bagl state that the frame belongs to.
 * @param frame Frame to destroy.
 */
void baglDestroyFrame(BaglState* state, BaglFrame** frame);

#endif