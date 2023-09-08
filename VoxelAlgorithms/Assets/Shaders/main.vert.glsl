#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vPosition;

layout(std140, binding = 0) uniform GlobalData {
   mat4 P;
   mat4 V;
   mat4 invVP;
   vec4 cameraPosition;
};

void main() {
  gl_Position = P * V * vec4(position, 1.0f);

  vNormal = normal;
  vPosition = position;
}