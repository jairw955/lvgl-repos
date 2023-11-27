#version 300 es
precision mediump float;
in float v_alpha;
in vec2 v_tex;
out vec4 color;
uniform sampler2D tex_in;
void main() {
  vec4 rgba = texture(tex_in, v_tex);
  color = rgba.bgra;
  color.a = rgba.a * v_alpha;
}

