#version 420 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

layout(binding = 3) uniform sampler2D color0;
layout(binding = 4) uniform sampler2D color1;
layout(binding = 5) uniform sampler2D color2;

void main() {
  vec4 sum = (texture(color0, uv) + texture(color1, uv) + texture(color2, uv));
  fragColor = sum / 3.0;
}