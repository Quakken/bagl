#include "foo.h"
#include "GLFW/glfw3.h"

#include "internal/stb_image.h"

int doThings() {
  glfwInit();

  uint8_t *imageData = stbi_load("asset.png", NULL, NULL, NULL, 0);
  return 0;
}