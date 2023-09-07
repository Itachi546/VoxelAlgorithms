#version 450

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vPosition;

void main() {
/*
  vec3 dx = dFdx(vPosition);
  vec3 dy = dFdy(vPosition);
  vec3 normal = normalize(cross(dx, dy));
  */

  vec3 col = vec3(0.0f);
  vec3 ld =	normalize(vec3(0.0,	1.0, 0.5));
  vec3 n = normalize(vNormal);
  vec3 diffuse = max(dot(n,	ld), 0.0f) * vec3(1.28,	1.20, 0.99);
  col += (n.y *	0.5	+ 0.5) * vec3(0.16,	0.20, 0.28);
  col += diffuse;
  
  col /= (1.0f + col);
  col = pow(col, vec3(0.4545));
  fragColor = vec4(col, 1.0f);
}