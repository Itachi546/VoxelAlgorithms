#version 460

#extension GL_GOOGLE_include_directive : enable

#include "noise.glsl"

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec4 vPosition;
layout(location = 2) in vec3 vCamPos;

vec3 getFractalNoise(vec3 p) {
    float f = 0.2;
    float a = 0.4;

    vec3 n = vec3(0.0);
    for (int i = 0; i < 3; ++i) {
        n += vec3(snoise(vPosition.xyz * f), snoise(vPosition.xzy * f), snoise(vPosition.yxz * f)) * a;
        f *= 2.0;
        a *= 0.5;
    }
    return n;
}

void main() {

  vec3 col = vec3(0.0f);
  vec3 ld =	normalize(vec3(0.5,	1.0, -0.5));
  vec3 n = vec3(0.0f);

#if 0
  n = normalize(vNormal);
#else
  vec3 dx = dFdx(vPosition.xyz);
  vec3 dy = dFdy(vPosition.xyz);
  vec3 normal = normalize(cross(dx, dy));
  n = normal;
 #endif

  vec3 diffuse = max(dot(n,	ld), 0.0f) * vec3(1.28,	1.20, 0.99);

  vec3 c1 = vec3(0.2, 0.5, 0.1);
  vec3 c0 = vec3(0.7, 0.3, 0.1);

  float petrub = snoise(vPosition.xyz * vec3(0.2, 1.0, 0.2)) * 8.0;
  vec3 albedo = mix(c0, c1, smoothstep(0.3, 1.0, n.y));

  col += (n.y * 0.5 + 0.5) * vec3(0.16, 0.20, 0.28);
  col += diffuse;// *vPosition.w * 0.5;
  col *= albedo;

  float fogDensity = 0.0005f;
  float heightFactor = 0.002f;

  vec3 fogOrigin = vCamPos;
  vec3 fogDirection = normalize(vPosition.xyz - vCamPos);  
  float fogDepth = dot(vCamPos - vPosition.xyz, vCamPos - vPosition.xyz);
  float fogAmount = heightFactor * exp(-fogOrigin.y * fogDensity) * 
                    (1.0f - exp(-fogDepth * fogDirection.y * fogDensity)) / fogDirection.y;

  fogAmount = clamp(fogAmount, 0.0f, 1.0f);
  vec3  fogColor  = vec3(0.5,0.6,0.7);

  col = mix( col, fogColor, fogAmount );
  col /= (1.0f + col);
  col = pow(col, vec3(0.4545));

  fragColor = vec4(col, 1.0f);
}