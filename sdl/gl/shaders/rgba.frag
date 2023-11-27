#version 300 es
precision mediump float;
in float v_alpha;
in vec2 v_tex;
out vec4 color;
uniform sampler2D tex_in;
void main() {
  vec2 tex_fix = vec2(v_tex.x, 1.0 - v_tex.y);
  vec4 rgba = texture(tex_in, tex_fix);
  color = rgba;
  color.a = rgba.a * v_alpha;
}
