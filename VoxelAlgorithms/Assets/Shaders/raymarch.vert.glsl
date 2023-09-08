#version 460

layout(location = 0) in vec2 position;

layout(location = 0) out vec2 vUV;

void main() {

  gl_Position = vec4(position, 0.0f, 1.0f);
  vUV = position;
}
