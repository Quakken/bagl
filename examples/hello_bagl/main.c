#include <stdlib.h>
#include <stdio.h>

#include "bagl/bagl.h"
#include "bagl/image.h"
#include "bagl/shader.h"
#include "bagl/window.h"
#include "bagl/frame.h"

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

  /* Fill frame with a pre-loaded image */
  BaglImage* image = baglLoadImage(state, "../assets/bagel.jpg");
  BaglFrameConfig cfg = {
      .numColorAttachments = 1,
      .colorAttachments = &image,
  };
  BaglFrame* frame = baglCreateFrame(state, &cfg);

  BaglShaderConfig shaderConfig = {
      /* Fullscreen VS is default */
      .fragmentFilename = "../assets/invert.frag",
  };
  BaglShader* invertEffect = baglCreateShader(state, &shaderConfig);

  /* Render loop */
  while (!baglShouldClose(state)) {
    baglPresent(state, frame, invertEffect);
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyShader(state, &invertEffect);
  baglDestroyImage(state, &image);
  baglDestroyFrame(state, &frame);
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