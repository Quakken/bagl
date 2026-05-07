#version 420 core

out vec4 color;

in VsOut {
  vec2 texCoords;
}
vsOut;

/* Materials */
layout(std140, binding = 1) uniform Material {
  float specularExponent;
  float transparency;
  vec3 ambient;
  vec3 specular;
};
layout(binding = 0) uniform sampler2D diffuseMap;
layout(binding = 1) uniform sampler2D specularMap;
layout(binding = 2) uniform sampler2D normalMap;

/* Lights */
// layout(std140, binding = 2) uniform Lights{};

void main() {
  vec3 diffuse = texture(diffuseMap, vsOut.texCoords).rgb;
  color = vec4(diffuse + ambient, 1.0);
}