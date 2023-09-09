version 430

#compute
// implementation from https://mrl.nyu.edu/~perlin/noise/

layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;

//output texture
//default binding point 0
layout(r32f, binding = 0) uniform image3D outputImage;

//Permutation array
//cannot be accessed using ivec4
layout(std140) uniform permutationBlock
{
  ivec4 permutation[64];
};



int p(int val)
{
  float fVal = floor(val / 4);
  float rem = val - fVal * 4;
  return permutation[int(fVal)][int(rem)];
}

vec3 fade(vec3 t) { 
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float lerp(float t, float a, float b) { 
  return a + t * (b - a); 
}

float grad(int hash, vec3 p) {
  int h = hash & 15;
  // Convert lower 4 bits of hash into 12 gradient directions
  float u = h < 8 ? p.x : p.y,
    v = h < 4 ? p.y : h == 12 || h == 14 ? p.x : p.z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float noise(vec3 p)
{

  // Find the unit cube that contains the point
  vec3 iP = floor(vec3(p));
  ivec3 P = ivec3(iP) & 255;

  // Find relative x, y,z of point in cube
  p -= iP;

  // Compute fade curves for each of x, y, z
  vec3 F = fade(p);
  
  // Hash coordinates of the 8 cube corners
  int A = p(P.x) + P.y;
  int AA = p(A) + P.z;
  int AB = p(A + 1) + P.z;
  int B = p(P.x + 1) + P.y;
  int BA = p(B) + P.z;
  int BB = p(B + 1) + P.z;

  // Add blended results from 8 corners of cube
  vec2 off = vec2(1.0f, 0.0f);
  float res = lerp(F.z,
		   lerp(F.y,
			lerp(F.x,
			     grad(p(AA), p),
			     grad(p(BA), p - off.xyy)),
			lerp(F.x,
			     grad(p(AB), p - off.yxy),
			     grad(p(BB), p - off.xxy))),
		   lerp(F.y,
			lerp(F.x,
			     grad(p(AA+1), p - off.yyx),
			     grad(p(BA+1), p - off.xyx)),
			lerp(F.x,
			     grad(p(AB+1), p - off.yxx),
			     grad(p(BB+1), p - off.xxx))));
  return (res + 1.0)/2.0;
}

uniform float noiseScale;
uniform int octaves;
uniform float persistance;
uniform float lacunarity;

vec2 iResolution = imageSize(outputImage);

void main()
{
  ivec3 fragCoord = ivec3(gl_GlobalInvocationID.xyz);
  float amp = 1.0f;
  float freq = 1.0f;
  float val = 0.0f;
  float total = 0.0f;
  vec2 p = fragCoord.xyz;
  for(int i = 0; i < octaves; ++i)
    {
      float noiseVal = noise(p) * 2.0f - 1.0f;
      val += amp * noiseVal;
      amp *= persistance;
      freq *= lacunarity;
    }

  vec4 col = vec4(val, 0.0f, 0.0f, 1.0f);
  imageStore(outputImage, fragCoord, col);
}
