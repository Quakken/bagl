#include "bagl/obj.h"
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

  BaglImage* image = baglLoadImage(state, "../assets/bagel.jpg");
  BaglMaterialConfig materialConfig = {
      .diffuseMap = image,
      .specularExponent = 1.0f,
  };
  BaglMaterial* material = baglCreateMaterial(state, &materialConfig);
  BaglModel* model = baglCreateModel(state, mesh, material);

  BaglCamera* camera = baglCreateCamera(state, NULL);
  baglSetCameraPosition(state, camera, 0, 0, 10);

  struct {
    float x, y, z;
  } camPos;
  camPos.x = camPos.y = camPos.z = 0.0f;
  float ticks = 0;

  baglLoadOBJ(state, "../assets/test.obj");

  /* Render loop */
  while (!baglShouldClose(state)) {
    /* Clear the frame */
    baglClearFrame(state, frame, BAGL_ATTACHMENT_COLOR, 0, 0, 0, 1);
    baglClearFrame(state, frame, BAGL_ATTACHMENT_DEPTH_STENCIL, 0, 0, 0, 0);

    /* Rotate the camera around the center of the scene */
    float s = sinf(ticks);
    float c = cosf(ticks);
    camPos.x = c * 5.0f;
    camPos.z = -s * 5.0f;
    float angle = -atan2f(camPos.x, camPos.z) * 180.0f / M_PI;
    baglSetCameraPosition(state, camera, camPos.x, camPos.y, camPos.z);
    baglSetCameraRotation(state, camera, 0, angle, 0);
    ticks += 0.025f;

    /* Rotate the box around the x axis and change scale */
    baglSetModelScale(state, model, 1.0f + fabsf(c), 1.0f + fabsf(c),
                      1.0f + fabsf(c));
    baglSetModelRotation(state, model, c * 180.0f, 0, 0);
    baglSetModelPosition(state, model, 0, s, 0);

    /* Draw the model */
    baglDraw(state, frame, model, camera);
    baglPresent(state, frame, NULL);
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyCamera(state, &camera);
  baglDestroyImage(state, &image);
  baglDestroyModel(state, &model);
  baglDestroyMaterial(state, &material);
  baglDestroyMesh(state, &mesh);
  baglDestroyFrame(state, &frame);
  baglDestroyState(&state);
  baglTerminate();

  /* Debugging: Print number of leaked allocations */
  if (allocs > 0) {
    printf("Memory leak detected! Allocations remaining: %d\n", allocs);
  }
}

void* trackedRealloc(void* data, size_t size) {
  if (!data && size > 0) {
    ++allocs;
  } else if (data && size == 0) {
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