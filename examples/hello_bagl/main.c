#include <stdlib.h>
#include <stdio.h>

#include "bagl/bagl.h"
#include "bagl/image.h"
#include "bagl/material.h"
#include "bagl/shader.h"
#include "bagl/window.h"
#include "bagl/frame.h"
#include "bagl/mesh.h"

/* Debugging: Track number of allocations made by the engine */
static int allocs = 0;
void* trackedRealloc(void* data, size_t size);
/* Debugging: Print all messages to standard output */
void printMessage(BaglLogLevel level, const char* message);

int main() {
  baglInit();

  BaglConfig config = {
      .logFn = printMessage,
      .reallocFn = trackedRealloc,
  };
  BaglState* state = baglCreateState(&config);
  /* Ensure state was created successfully */
  if (!state) {
    baglTerminate();
    exit(1);
  }

  /* Fill frame with a collection of pre-loaded images */
  BaglImage* images[] = {
      baglLoadImage(state, "../assets/bagel.jpg"),
      baglLoadImage(state, "../assets/pug.png"),
      baglLoadImage(state, "../assets/purrito.png"),
  };
  BaglFrameConfig cfg = {
      .numColorAttachments = 3,
      .colorAttachments = &images[0],
  };
  BaglFrame* imageFrame = baglCreateFrame(state, &cfg);
  BaglFrame* processedFrame = baglCreateFrame(state, NULL);

  /* Load shaders */
  BaglShaderConfig shaderConfig = {
      /* Fullscreen VS is default */
      .fragmentFilename = "../assets/invert.frag",
  };
  BaglShader* invertEffect = baglCreateShader(state, &shaderConfig);
  shaderConfig.fragmentFilename = "../assets/merge.frag";
  BaglShader* mergeEffect = baglCreateShader(state, &shaderConfig);

  /* Create a mesh */
  BaglVertex vertices[] = {
      /* Front */
      {{-0.5, -0.5, 0.5}, {0.0, 0.0, 1.0}, {0.0, 0.0}},
      {{0.5, -0.5, 0.5}, {0.0, 0.0, 1.0}, {1.0, 0.0}},
      {{0.5, 0.5, 0.5}, {0.0, 0.0, 1.0}, {1.0, 1.0}},
      {{-0.5, 0.5, 0.5}, {0.0, 0.0, 1.0}, {0.0, 1.0}},
      /* Back */
      {{0.5, -0.5, -0.5}, {0.0, 0.0, -1.0}, {0.0, 0.0}},
      {{-0.5, -0.5, -0.5}, {0.0, 0.0, -1.0}, {1.0, 0.0}},
      {{-0.5, 0.5, -0.5}, {0.0, 0.0, -1.0}, {1.0, 1.0}},
      {{0.5, 0.5, -0.5}, {0.0, 0.0, -1.0}, {0.0, 1.0}},
      /* Top */
      {{-0.5, 0.5, 0.5}, {0.0, 1.0, 0.0}, {0.0, 0.0}},
      {{0.5, 0.5, 0.5}, {0.0, 1.0, 0.0}, {1.0, 0.0}},
      {{0.5, 0.5, -0.5}, {0.0, 1.0, 0.0}, {1.0, 1.0}},
      {{-0.5, 0.5, -0.5}, {0.0, 1.0, 0.0}, {0.0, 1.0}},
      /* Bottom */
      {{-0.5, -0.5, -0.5}, {0.0, -1.0, 0.0}, {0.0, 0.0}},
      {{0.5, -0.5, -0.5}, {0.0, -1.0, 0.0}, {1.0, 0.0}},
      {{0.5, -0.5, 0.5}, {0.0, -1.0, 0.0}, {1.0, 1.0}},
      {{-0.5, -0.5, 0.5}, {0.0, -1.0, 0.0}, {0.0, 1.0}},
      /* Right */
      {{0.5, -0.5, 0.5}, {1.0, 0.0, 0.0}, {0.0, 0.0}},
      {{0.5, -0.5, -0.5}, {1.0, 0.0, 0.0}, {1.0, 0.0}},
      {{0.5, 0.5, -0.5}, {1.0, 0.0, 0.0}, {1.0, 1.0}},
      {{0.5, 0.5, 0.5}, {1.0, 0.0, 0.0}, {0.0, 1.0}},
      /* Left */
      {{-0.5, -0.5, -0.5}, {-1.0, 0.0, 0.0}, {0.0, 0.0}},
      {{-0.5, -0.5, 0.5}, {-1.0, 0.0, 0.0}, {1.0, 0.0}},
      {{-0.5, 0.5, 0.5}, {-1.0, 0.0, 0.0}, {1.0, 1.0}},
      {{-0.5, 0.5, -0.5}, {-1.0, 0.0, 0.0}, {0.0, 1.0}},
  };
  uint32_t indices[] = {
      0,  1,  2,  0,  2,  3,  /* Front */
      4,  5,  6,  4,  6,  7,  /* Back */
      8,  9,  10, 8,  10, 11, /* Top */
      12, 13, 14, 12, 14, 15, /* Bottom */
      16, 17, 18, 16, 18, 19, /* Right */
      20, 21, 22, 20, 22, 23, /* Left */
  };
  BaglMeshConfig meshConfig = {
      .numVertices = sizeof(vertices) / sizeof(BaglVertex),
      .vertices = &vertices[0],
      .numIndices = sizeof(indices) / sizeof(uint32_t),
      .indices = &indices[0],
  };
  BaglMesh* mesh = baglCreateMesh(state, &meshConfig);

  BaglMaterial* material = baglCreateMaterial(state, NULL);

  /* Render loop */
  while (!baglShouldClose(state)) {
    /* Combine post-processing effects (merge, then invert) */
    baglClearFrame(state, processedFrame, BAGL_ATTACHMENT_COLOR, 0, 0, 0, 1);
    baglProcessFrame(state, processedFrame, imageFrame, mergeEffect);
    baglPresent(state, processedFrame, invertEffect);
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyMaterial(state, &material);
  baglDestroyMesh(state, &mesh);
  baglDestroyShader(state, &mergeEffect);
  baglDestroyShader(state, &invertEffect);
  for (size_t i = 0; i < 3; ++i) {
    baglDestroyImage(state, &images[i]);
  }
  baglDestroyFrame(state, &imageFrame);
  baglDestroyFrame(state, &processedFrame);
  baglDestroyState(&state);
  baglTerminate();

  /* Debugging: Print number of leaked allocations */
  if (allocs > 0) {
    printf("Memory leak detected! Allocations remaining: %d\n", allocs);
  }
}

void* trackedRealloc(void* data, size_t size) {
  if (!data) {
    ++allocs;
  } else if (size == 0) {
    --allocs;
  }
  return realloc(data, size);
}

void printMessage(BaglLogLevel level, const char* message) {
  switch (level) {
    case BAGL_LOG_INFO:
      printf("\033[2m[bagl] INFO: ");
      break;
    case BAGL_LOG_STANDARD:
      printf("\033[2m[bagl] STANDARD: ");
      break;
    case BAGL_LOG_WARNING:
      printf("\033[33m[bagl] WARNING: ");
      break;
    case BAGL_LOG_ERROR:
      printf("\033[31m[bagl] ERROR: ");
      break;
    case BAGL_LOG_FATAL:
      printf("\033[41;37m[bagl] FATAL: ");
      break;
  }
  printf("%s\033[0m\n", message);
}