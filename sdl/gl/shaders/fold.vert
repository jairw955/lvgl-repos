in vec4 pos;
in vec2 tex;
uniform vec3 scale;
uniform vec3 offset;
uniform vec3 rot;
uniform vec3 move;
uniform vec3 t_scale;
uniform vec3 t_move;
uniform float alpha;
out vec2 v_tex;
out float v_alpha;
void main() {
  mat4 rotate = rotatey(rot.y);
  gl_Position = pos * move_xyz(offset)
    * scale_xyz(scale) * rotate * move_xyz(move);
  vec4 _tex = vec4(tex, 1.0, 1.0) * scale_xyz(t_scale)
    * move_xyz(t_move);
  v_tex = _tex.xy;
  v_alpha = alpha;
}

