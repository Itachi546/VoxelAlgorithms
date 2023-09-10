#version 460

#extension GL_GOOGLE_include_directive : enable
#include "noise.glsl"

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vPosition;

void main() {

  vec3 dx = dFdx(vPosition);
  vec3 dy = dFdy(vPosition);
  vec3 normal = normalize(cross(dx, dy));

  vec3 col = vec3(0.0f);
  vec3 ld =	normalize(vec3(0.5,	1.0, -0.5));
  vec3 n = vec3(0.0f);

#if 1
  n = normalize(vNormal);
#else
  n = normal;
  #endif

  vec3 diffuse = max(dot(n,	ld), 0.0f) * vec3(1.28,	1.20, 0.99);
  col += (n.y *	0.5	+ 0.5) * vec3(0.16,	0.20, 0.28);

  vec3 c1 = vec3(0.2, 0.5, 0.1);
  vec3 c0 = vec3(0.7, 0.3, 0.1);

  float petrub = snoise(vPosition.xyz * vec3(0.2, 1.0, 0.2)) * 8.0;
  vec3 albedo = mix(c0, c1, n.y * 0.5 + 0.5);

  if(n.y < 0.3)
   albedo *= mix(albedo, vec3(1.0, 0.1, 0.2), petrub);

  col += diffuse;

  col /= (1.0f + col);
  col =	pow(col, vec3(0.5));

  fragColor = vec4(col, 1.0f);
}