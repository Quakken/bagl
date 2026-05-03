#include <stdlib.h>
#include "bagl/bagl.h"

int main() {
  baglInit();

  BaglState* state = baglCreateState(NULL);
  /* Render objects */
  baglDestroyState(&state);

  baglTerminate();
}