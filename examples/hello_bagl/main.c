#include <stdlib.h>
#include <stdio.h>

#include "bagl/bagl.h"
#include "bagl/window.h"

int main() {
  baglInit();

  BaglState* state = baglCreateState(NULL);
  /* Ensure state was created successfully */
  if (!state) {
    baglTerminate();
    exit(1);
  }

  /* Render loop */
  while (!baglShouldClose(state)) {
    printf("Window dimensions: %dx%d. Viewport dimensions: %dx%d\n",
           baglGetWindowWidth(state), baglGetWindowHeight(state),
           baglGetViewportWidth(state), baglGetViewportHeight(state));
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyState(&state);
  baglTerminate();
}