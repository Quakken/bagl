#include "bagl/bagl.h"
#include "stdlib.h"

int main() {
  baglInit();

  BaglState* state = baglCreateState(NULL);
  /* Render objects */
  baglDestroyState(&state);

  baglTerminate();
}