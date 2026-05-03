#include <stdlib.h>
#include <stdio.h>

#include "bagl/bagl.h"
#include "bagl/window.h"
#include "bagl/frame.h"

int main() {
  baglInit();

  BaglState* state = baglCreateState(NULL);
  /* Ensure state was created successfully */
  if (!state) {
    baglTerminate();
    exit(1);
  }

  BaglFrame* frame = baglCreateFrame(state, NULL);

  /* Render loop */
  while (!baglShouldClose(state)) {
    baglClearFrame(frame, BAGL_ATTACHMENT_COLOR, 0.4, 0.4, 0.4, 1.0);
    baglPresent(state, frame, NULL);
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyFrame(state, &frame);
  baglDestroyState(&state);
  baglTerminate();
}