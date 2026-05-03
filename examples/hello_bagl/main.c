#include <stdlib.h>
#include "bagl/bagl.h"

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
    baglPollEvents();
  }

  /* Cleanup */
  baglDestroyState(&state);
  baglTerminate();
}