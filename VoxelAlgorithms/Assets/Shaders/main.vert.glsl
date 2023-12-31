#version 460

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec4 vPosition;
layout(location = 2) out vec3 vCamPos;

#extension GL_EXT_scalar_block_layout : enable

layout(std430, binding = 0) uniform GlobalData {
   mat4 P;
   mat4 V;
   mat4 invVP;
   vec4 cameraPosition;
};

layout(location = 0) uniform mat4 M;

void main() {
  vec4 worldPos = M * vec4(position.xyz, 1.0f);
  gl_Position = P * V * worldPos;

  vNormal = normal;
  vPosition = vec4(worldPos.xyz, position.w);
  vCamPos = cameraPosition.xyz;
}