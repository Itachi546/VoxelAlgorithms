#include "noise.glsl"

struct DensityParams {
    float amplitude;
	float frequency;
	float lacunarity;
	float persistence;
	int octaves;
};

layout(std140, binding = 8) uniform UniformParams {
   DensityParams densityParams;
};

float getDensity(vec3 p)
{
    //return min(p.y - 128.0f, length(p - 178.0f) - 50.0f);
   //return min(p.y - 15.0f, length(p - vec3(32.0f, 24.0f, 16.0f)) - 8.0f);
   float noise = 0.0f;
   float amplitude = densityParams.amplitude;//1.0f;
   float frequency = densityParams.frequency;//0.008f;
   float lacunarity = densityParams.lacunarity;//2.0f;
   float persistence = densityParams.persistence;//0.5f;
   int numOctaves = densityParams.octaves;//7;

   vec3 wp = p;
   float f = 0.02;
   float a = 4.0;
   //wp += vec3(snoise(p.xyz * f), snoise(p.xzy * f), snoise(p.yxz * f)) * a;
   for(int i = 0; i < numOctaves; ++i) {
      float n = snoise(wp * frequency);
      noise += n * amplitude;
      amplitude *= persistence;
      frequency *= lacunarity;
   }
   noise = clamp(noise * noise,	0.0, 1.0);
   float d = p.y - 128.0f + (noise * 2.0f - 1.0f) * 30.0f;
   return d;
}
