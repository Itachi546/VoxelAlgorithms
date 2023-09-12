#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#extension GL_GOOGLE_include_directive : require
#include "helpers.glsl"

struct Triangulation {
  int edges[16];
};

layout(std430, binding = 0) readonly buffer TriangleTable {
   Triangulation triangulations[];
};

layout(std430, binding = 1) writeonly buffer IndexBuffer {
  uint indices[];
};

layout(std430, binding = 2) writeonly buffer TriangleCount {
  uint totalTriangle;
};

layout(r32f, binding = 3) uniform image3D uNoiseTexture;
layout(std430, binding = 4) readonly buffer SplatBuffer {
   uvec4 splatBuffer[];
};

layout(location = 2) uniform int uVoxelCount;

float getDensity(ivec3 uv) {
  return imageLoad(uNoiseTexture, uv).r;
}

vec4 createPoint(ivec3 p) {
  return vec4(p.x, p.y, p.z, getDensity(p));
}

uint getVertexIndex(ivec3 uv, int edge) {
    ivec4 offset = EdgeToCellVertex[edge];
    uv += offset.xyz;

    int splatIndex = getSplatIndex(uv, uVoxelCount);
    return splatBuffer[splatIndex][offset.w];
}

void main() {
  
  ivec3 uv = ivec3(gl_GlobalInvocationID.xyz);
  if(uv.x > uVoxelCount || uv.y > uVoxelCount || uv.z > uVoxelCount) return; 

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

    if(edgeIndex == 0) return;

    Triangulation triangulation = triangulations[edgeIndex];
    for(int i = 0; i < 15; i += 3) {
       int e0 = triangulation.edges[i];
       if(e0 == -1) return;

       int e1 = triangulation.edges[i + 1];
       int e2 = triangulation.edges[i + 2];

       uint i0 = getVertexIndex(uv, e0);
       uint i1 = getVertexIndex(uv, e1);
       uint i2 = getVertexIndex(uv, e2);

       uint triangleIndex = atomicAdd(totalTriangle, 1);
       indices[triangleIndex * 3] = i0;
       indices[triangleIndex * 3 + 1] = i1;
       indices[triangleIndex * 3 + 2] = i2;
    }
}