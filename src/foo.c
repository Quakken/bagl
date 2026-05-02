#include "bagl/foo.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb/stb_image.h"

int doThings() {
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glfwInit();

  uint8_t *imageData = stbi_load("asset.png", NULL, NULL, NULL, 0);
  return 0;
}