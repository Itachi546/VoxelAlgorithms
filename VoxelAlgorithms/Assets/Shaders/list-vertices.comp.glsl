#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

const int VOXEL_COUNT = 256;

struct Vertex {
   float p[4];
   float n[3];
};

struct Triangulation {
  int edges[16];
};

layout(std430, binding = 0) readonly buffer TriangleTable {
   Triangulation triangulations[];
};

layout(std430, binding = 1) writeonly buffer Vertices {
  Vertex vertices[];
};

layout(std430, binding = 2) writeonly buffer VertexCount {
  uint totalTriangle;
};

layout(r32f, binding = 3) uniform image3D uNoiseTexture;

layout(std430, binding = 4) writeonly buffer SplatBuffer {
  uvec4 splatBuffer[];
};

float getDensity(ivec3 uv) {
  return imageLoad(uNoiseTexture, uv).r;
}

const int cornerAFromEdge[12] = int[12] (0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3);
const int cornerBFromEdge[12] = int[12] (1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7);

vec4 createPoint(ivec3 p) {
  return vec4(p.x, p.y, p.z, getDensity(p));
}

const ivec2 E = ivec2(1, 0);
float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

	return o4.y * d.y + o4.x *	(1.0 - d.y);
}

vec3 calculateNormal(vec3 p) {

   ivec3 ip = ivec3(p);
   return normalize(vec3(
      getDensity(ip + E.xyy) - getDensity(ip - E.xyy),
      getDensity(ip + E.yxy) - getDensity(ip - E.yxy),
      getDensity(ip + E.yyx) - getDensity(ip - E.yyx)
   ));
}

vec3 interpolatePosition(vec4 p0, vec4 p1, float isoLevel, out vec3 n) {
   float d0 = p0.w;
   float d1 = p1.w;

   float d = (isoLevel - d0)/ (d1 - d0);

   vec3 n0 = calculateNormal(p0.xyz);
   vec3 n1 = calculateNormal(p1.xyz);
   n = n0 + d * (n1 - n0); 
   
   vec3 p = p0.xyz + d * (p1.xyz - p0.xyz);
   vec3 v = vec3(noise(p * 4.0), noise(p.yxz * 4.0), noise(p.yzx * 4.0));
   // v += noise(p * 8.0) * 0.5;
   // v +=	noise(p	* 16.0)	* 0.25;
   n = normalize(n);

   return p;
}

void main() {
  
  ivec3 uv = ivec3(gl_GlobalInvocationID.xyz);

  // 8 corners of the current cube
  vec4 cubeCorners[8] = vec4[8](
        createPoint(ivec3(uv.x,     uv.y,     uv.z)),
        createPoint(ivec3(uv.x + 1, uv.y,     uv.z)),
        createPoint(ivec3(uv.x + 1, uv.y,     uv.z + 1)),
        createPoint(ivec3(uv.x,     uv.y,     uv.z + 1)),
        createPoint(ivec3(uv.x,     uv.y + 1, uv.z)),
        createPoint(ivec3(uv.x + 1, uv.y + 1, uv.z)),
        createPoint(ivec3(uv.x + 1, uv.y + 1, uv.z + 1)),
        createPoint(ivec3(uv.x, uv.y + 1, uv.z + 1))
    );

    const float isoLevel = 0.0f;

    int edgeIndex = 0;
    if(cubeCorners[0].w < isoLevel) edgeIndex |= 1;
    if(cubeCorners[1].w < isoLevel) edgeIndex |= 2;    // 1 << 1;
    if(cubeCorners[2].w < isoLevel) edgeIndex |= 4;    // 1 << 2;
    if(cubeCorners[3].w < isoLevel) edgeIndex |= 8;    // 1 << 3;
    if(cubeCorners[4].w < isoLevel) edgeIndex |= 16;   // 1 << 4;
    if(cubeCorners[5].w < isoLevel) edgeIndex |= 32;   // 1 << 5;
    if(cubeCorners[6].w < isoLevel) edgeIndex |= 64;   // 1 << 6;
    if(cubeCorners[7].w < isoLevel) edgeIndex |= 128;  // 1 << 7;

	if(edgeIndex ==	0) return;
    
    Triangulation triangulation = triangulations[edgeIndex];
    uvec3 values = uvec3(~0u);
    for(int i = 0; i < 15; ++i) {
       int e = triangulation.edges[i];
       // To reuse the vertex we only generate vertex if it lies on edge 0, 3, or 8
       if(e == -1) break;
       if(e != 0 && e != 3 && e != 8) continue; 
       if(values[e / 3] != ~0u) continue;

       vec3 n = vec3(0.0f);
   	   vec3 p = interpolatePosition(cubeCorners[cornerAFromEdge[e]], cubeCorners[cornerBFromEdge[e]], isoLevel, n);

       Vertex vertex;
       vertex.p[0] = p.x, vertex.p[1] = p.y, vertex.p[2] = p.z;
       vertex.p[3] = 0.0f;
       vertex.n[0] = n.x, vertex.n[1] = n.y, vertex.n[2] = n.z;


 	   uint	index =	atomicAdd(totalTriangle, 1);
       values[e / 3] = index;
       vertices[index] = vertex;
    }

    int splatIndex = uv.z * (VOXEL_COUNT + 1) * (VOXEL_COUNT + 1) + uv.y * (VOXEL_COUNT + 1) + uv.x;
    splatBuffer[splatIndex]	= uvec4(values, ~0u);
}