#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vPosition;

uniform mat4 P;
uniform mat4 V;

void main() {
  gl_Position = P * V * vec4(position, 1.0f);

  vNormal = normal;
  vPosition = position;
}