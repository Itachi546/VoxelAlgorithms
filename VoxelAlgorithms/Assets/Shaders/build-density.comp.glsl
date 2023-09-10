#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#extension GL_GOOGLE_include_directive : require
#include "noise.glsl"

layout(r32f, binding = 0) uniform image3D uDensityTexture;

float map(vec3 p)
{
   vec3 offset = vec3(0.0f);
   float noise = 0.0f;
   float amplitude = 1.0f;
   float frequency = 0.005f;
   float lacunarity = 2.0f;
   float persistence = 0.5f;
   int numOctaves = 7;

   for(int i = 0; i < numOctaves; ++i) {
      float n = snoise((p + offset) * frequency);
      noise += n * amplitude;
      amplitude *= persistence;
      frequency *= lacunarity;
   }
   noise = clamp(noise * noise, -1.0, 1.0);
   return p.y - 128.0f + noise * 70.0f;
}

void main() 
{
  ivec3 uv = ivec3(gl_GlobalInvocationID.xyz);
  float density = map(uv);
  imageStore(uDensityTexture, uv, vec4(density));
}

