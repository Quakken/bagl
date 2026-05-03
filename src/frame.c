#include "bagl/frame.h"

#include <stdbool.h>
#include <string.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "bagl/window.h"
#include "internal/bagl_state.h"
#include "internal/bagl_window.h"
#include "internal/utils.h"

const static BaglFrameConfig BAGL_DEFAULT_FRAME_CONFIG = {
    .numColorAttachments = 1,
    .enableDepthTest = true,
    .enableStencilTest = false,
};

typedef struct BaglFrame {
  GLuint fbo;

  int numColorAttachments;
  bool ownsColorAttachments;
  GLuint* colorAttachments; /* TODO: Make these pointers to bagl color
                               attachments/images instead */
  bool depthEnabled;
  bool stencilEnabled;
  bool ownsDepthStencilAttachment;
  GLuint depthStencilAttachment; /* TODO: Make this a pointer to a bagl
                                    depth/stencil buffer instead */
} BaglFrame;

BaglFrame* baglCreateFrame(BaglState* state, const BaglFrameConfig* config) {
  if (!state) {
    return NULL;
  }
  if (!config) {
    config = &BAGL_DEFAULT_FRAME_CONFIG;
  }

  /* Allocate the memory */
  BaglFrame* frame = state->reallocFn(NULL, sizeof(BaglFrame));
  if (!frame) {
    baglLog(state, ERROR, "Could not allocate bagl frame (in baglCreateFrame)");
    return NULL;
  }

  /* Create the framebuffer */
  glGenFramebuffers(1, &frame->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, frame->fbo);

  /* Initialize members */
  frame->numColorAttachments = config->numColorAttachments;
  frame->depthEnabled = config->enableDepthTest;
  frame->stencilEnabled = config->enableStencilTest;

  /* TODO: Make this not use GLuint */
  frame->colorAttachments =
      state->reallocFn(NULL, frame->numColorAttachments * sizeof(GLuint));
  if (!frame->colorAttachments) {
    baglLog(state, ERROR,
            "Color attachment allocation failed (in baglCreateFrame)");
    glDeleteFramebuffers(1, &frame->fbo);
    return NULL;
  }

  /* Generate attachments if not provided */
  if (config->numColorAttachments > 0 && !config->colorAttachments) {
    int viewportWidth = baglGetViewportWidth(state);
    int viewportHeight = baglGetViewportHeight(state);

    /* TODO: Actual color attachment creation here */
    glGenTextures(frame->numColorAttachments, frame->colorAttachments);
    for (size_t i = 0; i < (size_t)frame->numColorAttachments; ++i) {
      /* Initialize textures and bind to framebuffer */
      glBindTexture(GL_TEXTURE_2D, frame->colorAttachments[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth, viewportHeight, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    frame->ownsColorAttachments = true;
  } else {
    memcpy(frame->colorAttachments, (GLuint*)config->colorAttachments,
           frame->numColorAttachments * sizeof(*frame->colorAttachments));
    frame->ownsColorAttachments = false;
  }
  if ((config->enableDepthTest || config->enableStencilTest) &&
      !config->depthStencilAttachment) {
    int viewportWidth = baglGetViewportWidth(state);
    int viewportHeight = baglGetViewportHeight(state);

    glGenTextures(1, &frame->depthStencilAttachment);
    glBindTexture(GL_TEXTURE_2D, frame->depthStencilAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, viewportWidth,
                 viewportHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                 NULL);

    frame->ownsDepthStencilAttachment = true;
  } else {
    /* TODO */
    frame->depthStencilAttachment = 0;
    frame->ownsDepthStencilAttachment = false;
  }

  /* Bind attachments to the framebuffer */
  for (size_t i = 0; i < (size_t)frame->numColorAttachments; ++i) {
    glBindTexture(GL_TEXTURE_2D, frame->colorAttachments[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, frame->colorAttachments[i], 0);
  }
  glBindTexture(GL_TEXTURE_2D, frame->depthStencilAttachment);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, frame->depthStencilAttachment, 0);

  /* Make sure the framebuffer is complete */
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    baglDestroyFrame(state, &frame);
    baglLog(state, ERROR,
            "Frame was incomplete after creation (in baglCreateFrame)");
    return NULL;
  }

  baglLog(state, INFO, "Created frame");
  return frame;
}

void baglClearFrame(BaglFrame* frame,
                    BaglAttachment attachment,
                    float r,
                    float g,
                    float b,
                    float a) {
  if (!frame) {
    return;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, frame->fbo);
  glClearColor(r, g, b, a);
  switch (attachment) {
    case BAGL_ATTACHMENT_COLOR:
      glClear(GL_COLOR_BUFFER_BIT);
    case BAGL_ATTACHMENT_DEPTH_STENCIL:
      glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }
}

void baglPresent(BaglState* state, const BaglFrame* frame, void* shader) {
  if (!state || !frame) {
    return;
  }
  if (frame->numColorAttachments == 0) {
    baglLog(state, ERROR,
            "Can't present frame without color attachment (in baglPresent)");
    return;
  }
  if (!shader) {
    if (frame->numColorAttachments > 1) {
      baglLog(state, WARNING,
              "Can't present multiple color attachments without a shader. "
              "Presenting first color attachment. (in baglPresent)");
    }
    /* Get image bounds */
    // TODO: int width = baglGetImageWidth(frame->colorAttachments[0]);
    int width = baglGetViewportWidth(state);
    int height = baglGetViewportHeight(state);

    /* Just copy framebuffer contents */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frame->fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, baglGetViewportWidth(state),
                      baglGetViewportHeight(state), GL_COLOR_BUFFER_BIT,
                      GL_LINEAR);
  } else {
    /* TODO: Render image with shader (should be post-processing, vertex is full
     * screen quad) */
  }
  glfwSwapBuffers(state->window->window);
}

void baglDestroyFrame(BaglState* state, BaglFrame** frame) {
  if (!state || !frame || !(*frame)) {
    return;
  }
  BaglFrame* f = *frame;

  /* Free attachments */
  if (f->colorAttachments) {
    if (f->ownsColorAttachments) {
      glDeleteTextures(f->numColorAttachments, f->colorAttachments);
    }
    state->reallocFn(f->colorAttachments, 0);
  }
  if (f->ownsDepthStencilAttachment && f->depthStencilAttachment) {
    glDeleteTextures(1, &f->depthStencilAttachment);
  }

  glDeleteFramebuffers(1, &f->fbo);
  *(frame) = NULL;
  baglLog(state, INFO, "Destroyed frame");
}