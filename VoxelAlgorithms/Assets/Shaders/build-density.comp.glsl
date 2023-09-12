#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#extension GL_GOOGLE_include_directive : require
#include "noise.glsl"

layout(r32f, binding = 0) uniform image3D uDensityTexture;

layout(location = 1) uniform vec3 uOffset;

float map(vec3 p)
{
   p += uOffset;

   //return min(p.y - 15.0f, length(p - vec3(32.0f, 24.0f, 16.0f)) - 8.0f);
   float noise = 0.0f;
   float amplitude = 1.0f;
   float frequency = 0.008f;
   float lacunarity = 2.0f;
   float persistence = 0.5f;
   int numOctaves = 7;

   for(int i = 0; i < numOctaves; ++i) {
      float n = snoise((p) * frequency);
      noise += n * amplitude;
      amplitude *= persistence;
      frequency *= lacunarity;
   }
   //noise = clamp(noise * noise,	0.0, 1.0);
   float d = p.y - 128.0f + (noise * 2.0f - 1.0f) * 30.0f;
   return d;
}

void main() 
{
  ivec3 uv = ivec3(gl_GlobalInvocationID.xyz);

  float density = map(uv);
  imageStore(uDensityTexture, uv, vec4(density));
}

