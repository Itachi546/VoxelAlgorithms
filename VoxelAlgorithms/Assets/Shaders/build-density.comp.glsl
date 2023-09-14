#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#extension GL_GOOGLE_include_directive : require
#include "density.glsl"

layout(r32f, binding = 0) uniform image3D uDensityTexture;

layout(location = 1) uniform vec3 uOffset;

void main() 
{
  ivec3 uv = ivec3(gl_GlobalInvocationID.xyz);

  float density = getDensity(uv + uOffset);
  imageStore(uDensityTexture, uv, vec4(density));
}

