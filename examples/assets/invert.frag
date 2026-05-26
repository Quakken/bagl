#version 410 core

in vec2 uv;
out vec4 fragColor;

uniform sampler2D colorMaps[];

void main() {
  fragColor = vec4(1.0) - texture(colorMaps[0], uv);
}