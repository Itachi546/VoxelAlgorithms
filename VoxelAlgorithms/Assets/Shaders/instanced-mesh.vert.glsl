#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vPosition;
layout(location = 2) out vec3 vCamPos;
layout(location = 3) out vec3 vColor;

#extension GL_EXT_scalar_block_layout : enable

layout(std430, binding = 0) uniform GlobalData {
   mat4 P;
   mat4 V;
   mat4 invVP;
   vec4 cameraPosition;
};

vec3 COLORS[5] = vec3[5](
  vec3(1.0, 0.0, 0.0f),
  vec3(0.0, 1.0, 0.0f),
  vec3(0.0, 0.0, 1.0f),
  vec3(1.0, 0.0, 1.0f),
  vec3(1.0, 1.0, 0.0f)
);

vec3 getColor(int id) {
	id = (id << 3) * 49348 + (id << 2) * 3828 + (id << 1) * 9832;
	return vec3((id >> 8) / 255.0f, (id >> 16) / 255.0f, (id >> 24) / 255.0f);
}

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
  int id = gl_InstanceID % 5;
  vColor = COLORS[id];
}