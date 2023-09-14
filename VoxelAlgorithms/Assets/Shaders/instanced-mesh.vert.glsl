#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vPosition;
layout(location = 2) out vec3 vCamPos;

#extension GL_EXT_scalar_block_layout : enable

layout(std430, binding = 0) uniform GlobalData {
   mat4 P;
   mat4 V;
   mat4 invVP;
   vec4 cameraPosition;
};

struct Rigidbody {
	vec3 position;
	float mass;

	vec3 prevPosition;
	float restitution;

	vec3 velocity;
	float _pad1;
};

layout(binding = 1) buffer Rigidbodies {
   Rigidbody rbs[];
};

void main() {
  Rigidbody rb = rbs[gl_InstanceID];

  vec4 worldPos = vec4(position.xyz + rb.position, 1.0f);
  gl_Position = P * V * worldPos;

  vNormal = normal;
  vPosition = worldPos.xyz;
  vCamPos = cameraPosition.xyz;
}