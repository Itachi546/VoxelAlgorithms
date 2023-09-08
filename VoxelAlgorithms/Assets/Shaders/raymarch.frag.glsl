#version 460

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 vUV;

layout(binding = 0) uniform sampler3D uTexture;

uniform float uAspect;
uniform float uTime;
uniform int uStepCount;
uniform int uVoxelCount;

uniform mat4 uInverseVP;
uniform vec3 uCamPos;

vec3 MIN = -vec3((uVoxelCount - 1) * 0.5f);
vec3 MAX = vec3((uVoxelCount - 1) * 0.5f);
vec3 RANGE = vec3(uVoxelCount - 1);

bool intersectBox(vec3 r0, vec3 rd, out vec3 p0, out vec3 p1) {
   vec3 invDir = 1.0f / rd;

   vec3 tmin = MIN * abs(invDir) - r0 * invDir;
   vec3 tmax = MAX * abs(invDir) - r0 * invDir;

   float t0 = max(tmin.x, max(tmin.y, tmin.z));
   float t1 = min(tmax.x, min(tmax.y, tmax.z));

   if(t0 > t1 || t1 < 0.0f) return false;

   p0 = r0 + t0 * rd;
   p1 = r0 + t1 * rd;

   return true;
}

float signedDist(vec3 p) {
  p = (p - MIN) / RANGE;
  return texture(uTexture, p).r;
}

vec3 norm(vec3 p) {
  const vec2 e = vec2(1.0f, 0.0);
  return normalize(vec3(
     signedDist(p + e.xyy) -  signedDist(p - e.xyy),
     signedDist(p + e.yxy) -  signedDist(p - e.yxy),
     signedDist(p + e.yyx) -  signedDist(p - e.yyx)
  ));
}

float softShadow( in vec3 ro, in vec3 rd, float mint, float maxt, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<32 && t<maxt; i++ )
    {
        float h = signedDist(ro + rd*t);
        if( h<0.001 )
            return 0.0;
        res = min( res, k*h/t );
        t += h;
    }
    return res;
}

void main() {
  vec3 r0 = uCamPos;

  vec4 ndcCoord = vec4(vUV.x, vUV.y, 1.0f, 1.0f);
  vec4 worldCoord =	uInverseVP * ndcCoord;
  vec3 rd = normalize(worldCoord.xyz);

  vec3 col = vec3(0.0f);
  vec3 p0 = vec3(0.0f);
  vec3 p1 = vec3(0.0f);

  if(!intersectBox(r0, rd, p0, p1))	return;
  r0 = p0;
  float dist = length(p1 - p0);

  float d = 0.0f;
  for(int i = 0; i < uStepCount; ++i) {
     vec3 p = r0 + d * rd;  
     float t = signedDist(p);
     if(t < 0.1) 
     {

       vec3 ld = normalize(vec3(0.0, 1.0, 0.5));
       vec3 n = norm(p);
       col = n;

       vec3 diffuse = max(dot(n, ld), 0.0f) * vec3(1.28, 1.20, 0.99);
       col += (n.y * 0.5 + 0.5) * vec3(0.16, 0.20, 0.28);

       float shadow = 1.0f;//softShadow(p, ld, 0.3f, uVoxelCount, 30.0f);
       col += diffuse * shadow;
       break;
     }
     d += t;
     if(d > dist) break;
  }
  col /= (1.0f + col);
  col = pow(col, vec3(0.4545));
  fragColor = vec4(col, 1.0f);
}
