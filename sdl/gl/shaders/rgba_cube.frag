#version 300 es
precision mediump float;
in vec3 v_tex;
out vec4 color;
uniform samplerCube tex_in;
void main() {
  vec4 rgba = texture(tex_in, v_tex);
  color = rgba;
}

