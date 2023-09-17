#version 460

#extension GL_GOOGLE_include_directive : enable
#include "noise.glsl"

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vPosition;
layout(location = 2) in vec3 vCamPos;
layout(location = 3) in vec3 vColor;

void main() {

  vec3 col = vec3(0.0f);
  vec3 ld =	normalize(vec3(0.5,	1.0, -0.5));
  vec3 n = vec3(0.0f);

#if 1
  n = normalize(vNormal);
#else
  vec3 dx = dFdx(vPosition);
  vec3 dy = dFdy(vPosition);
  vec3 normal = normalize(cross(dx, dy));
  n = normal;
 #endif

  vec3 diffuse = max(dot(n,	ld), 0.0f) * vec3(1.28,	1.20, 0.99);
  col += (n.y *	0.5	+ 0.5) * vec3(0.16,	0.20, 0.28) * 4.;

  vec3 c1 = vec3(0.2, 0.5, 0.1);
  vec3 c0 = vec3(0.7, 0.3, 0.1);

  col += diffuse;

  float fogDensity = 0.0005f;
  float heightFactor = 0.002f;

  vec3 fogOrigin = vCamPos;
  vec3 fogDirection = normalize(vPosition - vCamPos);  
  float fogDepth = dot(vCamPos - vPosition, vCamPos - vPosition);
  float fogAmount = heightFactor * exp(-fogOrigin.y * fogDensity) * 
                    (1.0f - exp(-fogDepth * fogDirection.y * fogDensity)) / fogDirection.y;

  fogAmount = clamp(fogAmount, 0.0f, 1.0f);
  vec3  fogColor  = vec3(0.5,0.6,0.7);
  col = mix( col, fogColor, fogAmount );

  col /= (1.0f + col);
  col = pow(col, vec3(0.5));

  fragColor = vec4(col, 1.0f);
}