in vec4 pos;
in vec2 tex;
uniform vec3 offset;
uniform vec3 rot;
uniform vec3 vrot;
uniform vec3 scale;
uniform vec3 move;
uniform float alpha;
out vec2 v_tex;
out float v_alpha;

void main() {
  mat4 rotate = rotatex(rot.x) * rotatey(rot.y)
    * rotatez(rot.z);
  mat4 view = rotatex(vrot.x) * rotatey(vrot.y)
    * rotatez(vrot.z);
  gl_Position = pos * move_xyz(offset) * scale_xyz(scale)
    * rotate * view * perspective * move_xyz(move);
  v_tex = tex;
  v_alpha = alpha;
}

