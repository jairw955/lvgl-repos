in vec4 pos;
in vec3 tex;
uniform vec3 rot;
uniform vec3 vrot;
uniform vec3 scale;
uniform vec3 move;
out vec3 v_tex;

void main() {
  mat4 rotate = rotatex(rot.x) * rotatey(rot.y)
    * rotatez(rot.z);
  mat4 view = rotatex(vrot.x) * rotatey(vrot.y)
    * rotatez(vrot.z);
  gl_Position = pos * scale_xyz(scale) * rotate
    * view * move_xyz(move);
  v_tex = vec3(pos.x, pos.y, pos.z);
}
