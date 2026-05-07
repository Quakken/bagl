#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

out VsOut {
  vec2 texCoords;
}
vsOut;

/* Matrices */
layout(std140, binding = 0) uniform Matrices {
  mat4 view;
  mat4 projection;
};
uniform mat4 model;

void main() {
  gl_Position = /* projection * view * */ model * vec4(position, 1.0);
  vsOut.texCoords = texCoords;
}