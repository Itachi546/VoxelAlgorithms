#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vPosition;

#extension GL_EXT_scalar_block_layout : enable

layout(std430, binding = 0) uniform GlobalData {
   mat4 P;
   mat4 V;
   mat4 invVP;
   vec4 cameraPosition;
};

layout(std430, binding = 1) uniform ModelMatrix {
   mat4 M;
};

void main() {
  gl_Position = P * V * M * vec4(position.xyz, 1.0f);

  vNormal = normal;
  vPosition = position;
}