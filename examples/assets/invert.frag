#version 410 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D color0;

void main() {
  fragColor = vec4(1.0) - texture(color0, uv);
}