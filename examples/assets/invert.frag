#version 410 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D colorMaps[];

uniform float time;

void main() {
  fragColor = vec4(sin(time) * 0.5 + 0.5) - texture(colorMaps[0], uv);
}