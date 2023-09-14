#version 460

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

struct Rigidbody {
	vec3 position;
	float mass;

	vec3 prevPosition;
	float restitution;

	vec3 velocity;
	float _pad1;
};

layout(binding = 0) buffer Rigidbodies {
  Rigidbody rbs[];
};

layout(location = 0) uniform int uRBCount;
layout(location = 1) uniform float uDt;

#extension GL_GOOGLE_include_directive : require
#include "density.glsl"

const float GRAVITY = -4.0f;

const ivec2 E = ivec2(1, 0);
vec3 calculateNormal(vec4 p) {
   ivec3 ip = ivec3(p.xyz);
   return normalize(vec3(
      getDensity(ip + E.xyy) - p.w,
      getDensity(ip + E.yxy) - p.w,
      getDensity(ip + E.yyx) - p.w
   ));
}

vec3 interpolatePosition(vec4 p0, vec4 p1, float isoLevel, out vec3 n) {
   float d0 = p0.w;
   float d1 = p1.w;

   float d = (isoLevel - d0)/ (d1 - d0);

   vec3 n0 = calculateNormal(p0);
   vec3 n1 = calculateNormal(p1);
   n = n0 + d * (n1 - n0); 
   
   vec3 p = p0.xyz + d * (p1.xyz - p0.xyz);

   n = normalize(n);

   return p;
}

void main() {
    int id = int(gl_GlobalInvocationID.x);
    if(id >= uRBCount) return;

	vec3 p0 = rbs[id].prevPosition;
	vec3 p1 = rbs[id].position;
	vec3 velocity = rbs[id].velocity;
	float invMass = 1.0f / rbs[id].mass;

	int velocitySteps = 1;
	float dt = uDt / float(velocitySteps);

    float radius = 1.0f;
	for(int i = 0; i < velocitySteps; ++i) {
		vec3 v = normalize(velocity);
    	float d0 = getDensity(p0 + v * radius);
	    float d1 = getDensity(p1 + v * radius);

		if(d0 >	0.0f &&	d1 < 0.0f) {
           // We have collision
     	   float isoLevel =	0.0f;

		   // Calculate	the	contact	point and normal
		   vec3	n =	vec3(0.0f);
		   vec3	contactPoint = interpolatePosition(vec4(p0, d0), vec4(p1, d1), isoLevel, n);

		   float penetrationDistance = radius -	length(contactPoint	- p1);
		   p1 += penetrationDistance * n;
		   velocity	= reflect(rbs[id].velocity, n)	* 0.1;
	    }

		p0 = p1;
		velocity +=	vec3(0.0f, GRAVITY,	0.0f) *	invMass * dt;
        p1 += rbs[id].velocity * dt;
	}

    rbs[id].prevPosition = p0;
	rbs[id].position = p1;
	rbs[id].velocity = velocity;
}