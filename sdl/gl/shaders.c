static const char *_vert =
    "#version 300 es\n"
    "in vec4 pos;\n"
    "in vec2 tex;\n"
    "uniform mat4 view;\n"
    "uniform mat4 scale;\n"
    "uniform mat4 move;\n"
    "uniform mat4 perspective;\n"
    "out vec2 v_tex;\n"
    "void main() {\n"
    "  gl_Position = pos * scale * view * move;\n"
    "  v_tex = tex;\n"
    "}\n";

static const char *_vert_cube =
    "#version 300 es\n"
    "in vec4 pos;\n"
    "in vec3 tex;\n"
    "uniform mat4 view;\n"
    "uniform mat4 scale;\n"
    "uniform mat4 move;\n"
    "uniform mat4 perspective;\n"
    "out vec3 v_tex;\n"
    "void main() {\n"
    "  gl_Position = pos * scale * view * move;\n"
    "  v_tex = vec3(-pos.x, pos.y, pos.z);\n"
    "}\n";
#if 0
	"attribute vec4 pos;\n"
    "attribute vec2 tex;\n"
    "varying vec2 v_tex;\n"
    "void main() {\n"
    "  gl_Position = pos;\n"
    "  v_tex = tex;\n"
    "}\n";
#endif
static const char *_vert_fold =
    "#version 300 es\n"
    "in vec4 pos;\n"
    "in vec2 tex;\n"
    "uniform mat4 rotate;\n"
    "uniform mat4 scale;\n"
    "uniform mat4 move;\n"
    "uniform mat4 offset;\n"
    "uniform mat4 t_move;\n"
    "uniform mat4 view;\n"
    "out vec2 v_tex;\n"
    "void main() {\n"
    "  gl_Position = pos * scale * offset * rotate * move * view;\n"
    "  vec4 _tex = vec4(tex, 1.0, 1.0) * scale * t_move;"
    "  v_tex = _tex.xy;\n"
    "}\n";

static const char *_frag_rgba =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_tex;\n"
    "out vec4 color;\n"
    "uniform sampler2D tex_in;\n"
    "void main() {\n"
    "  vec2 tex_fix = vec2(v_tex.x, 1.0 - v_tex.y);\n"
    "  vec4 rgba = texture2D(tex_in, tex_fix);\n"
    "  color = rgba;\n"
    "}\n";
static const char *_frag_bgra =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_tex;\n"
    "out vec4 color;\n"
    "uniform sampler2D tex_in;\n"
    "void main() {\n"
    "  vec2 tex_fix = vec2(v_tex.x, 1.0 - v_tex.y);\n"
    "  vec4 rgba = texture2D(tex_in, v_tex);\n"
    "  color = rgba.bgra;\n"
    "}\n";
static const char *_frag_rgba_cube =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec3 v_tex;\n"
    "out vec4 color;\n"
    "uniform samplerCube tex_in;\n"
    "void main() {\n"
    "  vec4 rgba = texture(tex_in, v_tex);\n"
    "  color = rgba;\n"
    "}\n";
static const char *_frag_bgra_cube =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec3 v_tex;\n"
    "out vec4 color;\n"
    "uniform samplerCube tex_in;\n"
    "void main() {\n"
    "  vec4 rgba = texture(tex_in, v_tex);\n"
    "  color = rgba.bgra;\n"
    "}\n";
#if 0
    "precision mediump float;\n"
    "varying vec2 v_tex;\n"
    "uniform sampler2D ourTexture;\n"
    "void main() {\n"
    "  vec4 rgba = texture2D(ourTexture, v_tex);\n"
    "  gl_FragColor = rgba;\n"
    "  gl_FragColor.r = rgba.b;\n"
    "  gl_FragColor.b = rgba.r;\n"
    "}\n";
#endif
