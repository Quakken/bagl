#version 420 core

out vec4 color;

in VsOut {
  vec3 normal;
}
vsOut;

void main() {
  color = vec4(vsOut.normal / 2.0 + vec3(0.5), 1.0);
}