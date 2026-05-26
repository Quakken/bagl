#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "bagl/bagl.h"
#include "bagl/image.h"
#include "bagl/material.h"
#include "bagl/model.h"
#include "bagl/shader.h"
#include "bagl/window.h"
#include "bagl/frame.h"
#include "bagl/mesh.h"
#include "bagl/camera.h"

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

  BaglFrame* frame = baglCreateFrame(state, NULL);

  /* Load a model */
  BaglModel* gnome = baglLoadModel(state, "../assets/garden_gnome_1k.gltf");
  baglSetModelScale(state, gnome, 2, 2, 2);
  baglSetModelPosition(state, gnome, 0, -0.5, 0);

  BaglCamera* camera = baglCreateCamera(state, NULL);
  baglSetCameraPosition(state, camera, 0, 0, 10);

  BaglShaderConfig invertCfg = {
      .fragmentFilename = "../assets/invert.frag",
  };
  BaglShader* invert = baglCreateShader(state, &invertCfg);

  struct {
    float x, y, z;
  } camPos;
  camPos.x = camPos.y = camPos.z = 0.0f;
  float ticks = 0;

  /* Render loop */
  while (!baglShouldClose(state)) {
    /* Clear the frame */
    baglClearFrame(state, frame, BAGL_ATTACHMENT_COLOR, 0, 0, 0, 1);
    baglClearFrame(state, frame, BAGL_ATTACHMENT_DEPTH_STENCIL, 0, 0, 0, 0);

    /* Rotate the camera around the center of the scene */
    float s = sinf(ticks);
    float c = cosf(ticks);
    camPos.x = c * 2.0f;
    camPos.z = -s * 2.0f;
    float angle = -atan2f(camPos.x, camPos.z) * 180.0f / M_PI;
    baglSetCameraPosition(state, camera, camPos.x, camPos.y, camPos.z);
    baglSetCameraRotation(state, camera, 0, angle, 0);
    ticks += 0.00025f;

    /* Draw the model */
    baglDraw(state, frame, gnome, camera);
    baglPresent(state, frame, invert);
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyShader(state, &invert);
  baglDestroyModel(state, &gnome);
  baglDestroyCamera(state, &camera);
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