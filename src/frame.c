#include "bagl/frame.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "glad/glad.h" /* OpenGL */
#include "GLFW/glfw3.h"

#include "bagl/image.h" /* BaglImage, baglCreateImage */

#include "internal/bagl_mesh.h"     /* BaglMesh */
#include "internal/bagl_material.h" /* BaglMaterial */
#include "internal/bagl_model.h"    /* BaglModel */
#include "internal/bagl_shader.h"   /* BaglShader, texture units */
#include "internal/bagl_image.h"    /* BaglImage */
#include "internal/bagl_state.h"    /* BaglState */
#include "internal/bagl_window.h"   /* BaglWindow */
#include "internal/bagl_camera.h"   /* BaglCamera */
#include "internal/utils.h"         /* baglLog */

/* Default configuration used to create a frame */
const static BaglFrameConfig BAGL_DEFAULT_FRAME_CONFIG = {
    .numColorAttachments = 1,
    .enableDepthTest = true,
    .enableStencilTest = false,
};

typedef struct BaglFrame {
  size_t numColorAttachments;
  BaglImage** colorAttachments;
  BaglImage* depthStencilAttachment;

  GLuint fbo;

  bool ownsColorAttachments;
  bool ownsDepthStencilAttachment;

  bool depthEnabled;
  bool stencilEnabled;
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
    baglLog(state, ERROR, "Could not allocate frame (in baglCreateFrame)");
    return NULL;
  }

  /* Create the framebuffer */
  glGenFramebuffers(1, &frame->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, frame->fbo);

  /* Initialize members */
  frame->numColorAttachments = config->numColorAttachments;
  frame->depthEnabled = config->enableDepthTest;
  frame->stencilEnabled = config->enableStencilTest;

  frame->colorAttachments =
      state->reallocFn(NULL, frame->numColorAttachments * sizeof(BaglImage*));
  if (!frame->colorAttachments) {
    baglLog(state, ERROR,
            "Could not allocate color attachment storage (in baglCreateFrame)");
    glDeleteFramebuffers(1, &frame->fbo);
    return NULL;
  }

  /* Generate attachments if not provided */
  if (config->numColorAttachments > 0 && !config->colorAttachments) {
    BaglImageConfig config = {
        .width = baglGetViewportWidth(state),
        .height = baglGetViewportHeight(state),
        .minFilter = BAGL_FILTER_LINEAR,
        .magFilter = BAGL_FILTER_LINEAR,
        .format = BAGL_FORMAT_RGBA,
        .data = NULL,
    };
    for (size_t i = 0; i < frame->numColorAttachments; ++i) {
      frame->colorAttachments[i] = baglCreateImage(state, &config);
    }
    frame->ownsColorAttachments = true;
  } else {
    /* Copy attachments */
    if (config->colorAttachments) {
      memcpy(frame->colorAttachments, config->colorAttachments,
             frame->numColorAttachments * sizeof(BaglImage*));
    } else {
      frame->colorAttachments = NULL;
    }
    frame->ownsColorAttachments = false;
  }
  if ((config->enableDepthTest || config->enableStencilTest) &&
      !config->depthStencilAttachment) {
    BaglImageConfig config = {
        .width = baglGetViewportWidth(state),
        .height = baglGetViewportHeight(state),
        .minFilter = BAGL_FILTER_NEAREST,
        .magFilter = BAGL_FILTER_NEAREST,
        .format = BAGL_FORMAT_DEPTH_STENCIL,
        .data = NULL,
    };
    frame->depthStencilAttachment = baglCreateImage(state, &config);
    frame->ownsDepthStencilAttachment = true;
  } else {
    frame->depthStencilAttachment = config->depthStencilAttachment;
    frame->ownsDepthStencilAttachment = false;
  }

  /* Bind attachments to the framebuffer */
  for (size_t i = 0; i < (size_t)frame->numColorAttachments; ++i) {
    /* Ensure color attachment formats are valid */
    if (frame->colorAttachments[i]->format != BAGL_FORMAT_RGBA &&
        frame->colorAttachments[i]->format != BAGL_FORMAT_RGB) {
      baglLog(state, WARNING,
              "Color attachment has invalid format (in baglCreateFrame)");
    }
    /* Attach to framebuffer */
    glBindTexture(GL_TEXTURE_2D, frame->colorAttachments[i]->texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, frame->colorAttachments[i]->texture,
                           0);
  }
  if (frame->depthStencilAttachment) {
    /* Ensure depth/stencil attachment format is valid */
    if (frame->depthStencilAttachment->format != BAGL_FORMAT_DEPTH_STENCIL) {
      baglLog(
          state, WARNING,
          "Depth/stencil attachment has invalid format (in baglCreateFrame)");
    }

    glBindTexture(GL_TEXTURE_2D, frame->depthStencilAttachment->texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                           GL_TEXTURE_2D,
                           frame->depthStencilAttachment->texture, 0);
  }

  /* Make sure the framebuffer is complete */
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    baglDestroyFrame(state, &frame);
    baglLog(state, ERROR,
            "Frame was incomplete after creation (in baglCreateFrame)");
    return NULL;
  }

  baglLog(state, INFO, "Frame created");
  return frame;
}

void baglClearFrame(BaglState* state,
                    BaglFrame* frame,
                    BaglAttachment attachment,
                    float r,
                    float g,
                    float b,
                    float a) {
  if (!state || !frame) {
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

void baglProcessFrame(BaglState* state,
                      BaglFrame* dest,
                      const BaglFrame* src,
                      BaglShader* shader) {
  if (!state || !dest || !src || !shader) {
    return;
  }
  if (dest == src) {
    baglLog(state, ERROR,
            "Destination and source frames cannot be the same (in "
            "baglProcessFrame)");
    return;
  }
  /* Bind required sources */
  glBindVertexArray(state->emptyVAO);
  glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
  glUseProgram(shader->program);
  /* Bind all color attachments as textures */
  for (size_t i = 0; i < src->numColorAttachments; ++i) {
    glActiveTexture(GL_TEXTURE0 + BAGL_COLOR_TEXTURE_UNIT + i);
    glBindTexture(GL_TEXTURE_2D, src->colorAttachments[i]->texture);
  }
  /* Draw to the framebuffer */
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

void baglDrawModelMesh(BaglState* state,
                       BaglFrame* frame,
                       BaglModel* model,
                       BaglMesh* mesh,
                       BaglMaterial* material,
                       BaglCamera* camera) {
  /* TODO: Stencil tests */
  glUseProgram(material->shader->program);
  /* Bind all material data */
  glBindBufferBase(GL_UNIFORM_BUFFER, BAGL_MATERIAL_BINDING, material->ubo);
  if (material->diffuseMap) {
    glActiveTexture(GL_TEXTURE0 + BAGL_DIFFUSE_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, material->diffuseMap->texture);
  }
  if (material->specularMap) {
    glActiveTexture(GL_TEXTURE0 + BAGL_SPECULAR_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, material->specularMap->texture);
  }
  if (material->normalMap) {
    glActiveTexture(GL_TEXTURE0 + BAGL_NORMAL_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, material->normalMap->texture);
  }
  /* Update and bind camera matrices */
  if (camera->isViewDirty) {
    baglUpdateCameraMatrices(state, camera);
  }
  glBindBuffer(GL_UNIFORM_BUFFER, camera->ubo);
  glBindBufferBase(GL_UNIFORM_BUFFER, BAGL_MATRIX_BINDING, camera->ubo);

  /* Bind model matrix */
  if (model->isTransformDirty) {
    baglGenTransform(&model->transform[0], &model->transformConfig);
    model->isTransformDirty = false;
  }
  glUniformMatrix4fv(
      glGetUniformLocation(material->shader->program, BAGL_MODEL_UNIFORM_NAME),
      1, false, &model->transform[0]);

  /* Draw the model */
  glBindVertexArray(mesh->vao);
  glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, NULL);
}

void baglDraw(BaglState* state,
              BaglFrame* frame,
              BaglModel* model,
              BaglCamera* camera) {
  if (!state || !frame || !model || !camera) {
    return;
  }
  /* Enable depth testing */
  if (frame->depthEnabled) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  /* Resize the viewport */
  if (frame->numColorAttachments >= 1) {
    glViewport(0, 0, frame->colorAttachments[0]->width,
               frame->colorAttachments[0]->height);
  } else if (frame->depthStencilAttachment) {
    glViewport(0, 0, frame->depthStencilAttachment->width,
               frame->depthStencilAttachment->height);
  }

  /* Draw the meshes */
  glBindFramebuffer(GL_FRAMEBUFFER, frame->fbo);
  for (size_t i = 0; i < model->numMeshes; ++i) {
    BaglMesh* mesh = model->meshes[i];
    BaglMaterial* material = model->materials[mesh->materialIdx];
    baglDrawModelMesh(state, frame, model, mesh, material, camera);
  }
  glBindVertexArray(0);

  /* Resize the viewport */
  glViewport(0, 0, state->window->baseWidth, state->window->baseHeight);
}

void baglPresent(BaglState* state, const BaglFrame* frame, BaglShader* shader) {
  if (!state || !frame) {
    return;
  }
  if (frame->numColorAttachments == 0) {
    baglLog(state, ERROR,
            "Can't present frame without color attachments (in baglPresent)");
    return;
  }
  if (!shader) {
    if (frame->numColorAttachments > 1) {
      baglLog(state, WARNING,
              "Can't present multiple color attachments without a shader. "
              "Presenting first color attachment. (in baglPresent)");
    }
    /* Get image bounds */
    int width = frame->colorAttachments[0]->width;
    int height = frame->colorAttachments[0]->height;

    /* Just copy framebuffer contents */
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frame->fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, baglGetViewportWidth(state),
                      baglGetViewportHeight(state), GL_COLOR_BUFFER_BIT,
                      GL_LINEAR);
  } else {
    /* Bind resources */
    glBindVertexArray(state->emptyVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader->program);
    /* Bind all color attachments as textures */
    for (size_t i = 0; i < frame->numColorAttachments; ++i) {
      glActiveTexture(GL_TEXTURE0 + BAGL_COLOR_TEXTURE_UNIT + i);
      glBindTexture(GL_TEXTURE_2D, frame->colorAttachments[i]->texture);
    }
    /* Draw to the framebuffer */
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
  }
  glfwSwapBuffers(state->window->window);
}

BaglImage** baglGetColorAttachments(BaglState* state,
                                    const BaglFrame* frame,
                                    size_t* count) {
  if (!state || !count) {
    return NULL;
  }
  if (!frame) {
    *count = 0;
    return NULL;
  }
  *count = frame->numColorAttachments;
  return frame->colorAttachments;
}

BaglImage* baglGetDepthStencilAttachment(BaglState* state,
                                         const BaglFrame* frame) {
  if (!frame) {
    return NULL;
  }
  return frame->depthStencilAttachment;
}

void baglSetDepthTestEnabled(BaglState* state, BaglFrame* frame, bool enabled) {
  if (!frame) {
    return;
  }
  frame->depthEnabled = enabled;
}

void baglSetStencilTestEnabled(BaglState* state,
                               BaglFrame* frame,
                               bool enabled) {
  if (!state || !frame) {
    return;
  }
  frame->stencilEnabled = enabled;
}

void baglDestroyFrame(BaglState* state, BaglFrame** frame) {
  if (!state || !frame || !(*frame)) {
    return;
  }
  BaglFrame* f = *frame;

  /* Free attachments */
  if (f->colorAttachments) {
    if (f->ownsColorAttachments) {
      for (size_t i = 0; i < f->numColorAttachments; ++i) {
        baglDestroyImage(state, &f->colorAttachments[i]);
      }
    }
    state->reallocFn(f->colorAttachments, 0);
  }
  if (f->ownsDepthStencilAttachment && f->depthStencilAttachment) {
    baglDestroyImage(state, &f->depthStencilAttachment);
  }

  glDeleteFramebuffers(1, &f->fbo);

  /* Release frame memory */
  state->reallocFn(f, 0);
  *frame = NULL;

  baglLog(state, INFO, "Frame destroyed");
}