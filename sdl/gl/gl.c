#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#if USE_SDL_OPENGL
#include <lvgl/lvgl.h>

#define SDL_MAIN_HANDLED
#include <GLES3/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_egl.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengles2.h>

#include "gl.h"

#define DATA_PATH       "/usr/share/lv_drivers/shaders"

static char *_common = DATA_PATH"/common.vert";
static char *_vert = DATA_PATH"/default.vert";
static char *_vert_cube = DATA_PATH"/cube.vert";
static char *_vert_fold = DATA_PATH"/fold.vert";
static char *_frag_bgra = DATA_PATH"/bgra.frag";
static char *_frag_rgba = DATA_PATH"/rgba.frag";
static char *_frag_bgra_cube = DATA_PATH"/bgra_cube.frag";
static char *_frag_rgba_cube = DATA_PATH"/rgba_cube.frag";

static const GLfloat verts_2d[] = {
    -1.0, -1.0, 0.0, 1.0, 0.0, 1.0,
     1.0, -1.0, 0.0, 1.0, 1.0, 1.0,
    -1.0,  1.0, 0.0, 1.0, 0.0, 0.0,
     1.0,  1.0, 0.0, 1.0, 1.0, 0.0,
};

static const GLfloat verts_cube[] = {
    /* Front */
    -1.0, -1.0,  1.0, 1.0, 0.0, 0.0,
     1.0, -1.0,  1.0, 1.0, 1.0, 0.0,
    -1.0,  1.0,  1.0, 1.0, 0.0, 1.0,
    -1.0,  1.0,  1.0, 1.0, 0.0, 1.0,
     1.0,  1.0,  1.0, 1.0, 1.0, 1.0,
     1.0, -1.0,  1.0, 1.0, 0.0, 1.0,

    /* Left */
    -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    -1.0, -1.0,  1.0, 1.0, 1.0, 0.0,
    -1.0,  1.0, -1.0, 1.0, 0.0, 1.0,
    -1.0,  1.0, -1.0, 1.0, 0.0, 1.0,
    -1.0,  1.0,  1.0, 1.0, 1.0, 1.0,
    -1.0, -1.0,  1.0, 1.0, 0.0, 1.0,

    /* Right */
     1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
     1.0, -1.0,  1.0, 1.0, 1.0, 0.0,
     1.0,  1.0,  1.0, 1.0, 0.0, 1.0,
     1.0,  1.0,  1.0, 1.0, 0.0, 1.0,
     1.0,  1.0, -1.0, 1.0, 1.0, 1.0,
     1.0, -1.0, -1.0, 1.0, 0.0, 1.0,

    /* Back */
    -1.0,  1.0, -1.0, 1.0, 0.0, 0.0,
    -1.0, -1.0, -1.0, 1.0, 1.0, 0.0,
     1.0, -1.0, -1.0, 1.0, 0.0, 1.0,
     1.0, -1.0, -1.0, 1.0, 0.0, 1.0,
     1.0,  1.0, -1.0, 1.0, 1.0, 1.0,
    -1.0,  1.0, -1.0, 1.0, 0.0, 1.0,

    /* Top */
    -1.0,  1.0,  1.0, 1.0, 0.0, 0.0,
     1.0,  1.0,  1.0, 1.0, 1.0, 0.0,
    -1.0,  1.0, -1.0, 1.0, 0.0, 1.0,
    -1.0,  1.0, -1.0, 1.0, 0.0, 1.0,
     1.0,  1.0, -1.0, 1.0, 1.0, 1.0,
     1.0,  1.0,  1.0, 1.0, 0.0, 1.0,

    /* Bottom */
    -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    -1.0, -1.0,  1.0, 1.0, 1.0, 0.0,
     1.0, -1.0, -1.0, 1.0, 0.0, 1.0,
     1.0, -1.0, -1.0, 1.0, 0.0, 1.0,
     1.0, -1.0,  1.0, 1.0, 1.0, 1.0,
    -1.0, -1.0,  1.0, 1.0, 0.0, 1.0,
};

enum {
    VERTEX_DEFAULT,
    VERTEX_CUBE,
    VERTEX_FOLD,
};

enum {
    IMAGESOURCE_RGBA,
    IMAGESOURCE_BGRA,
};

typedef struct {
    GLuint program;
    GLuint vert;
    GLuint frag;
    GLuint VAO;
    GLuint VBO;
} lv_gl_prog_t;

typedef struct {
    lv_gl_base_t base;
    int inited;
    lv_gl_prog_t * cur_prog;
    lv_gl_prog_t * progs[6];
    SDL_Window * window;
    SDL_GLContext context;
} lv_gl_ctx_t;

static lv_gl_ctx_t gl_ctx = {
    .inited = 0,
};

static GLuint gl_pos = 0;
static GLuint gl_tex = 1;

static void reset_crop(lv_gl_base_t *base);
static void reset_viewport(lv_gl_base_t *base);
static void update_crop_viewport(lv_gl_base_t *base);

static GLuint create_shader(const char *source_path, GLenum shader_type)
{
    GLuint shader;
    GLint status;
    FILE *fd;
    FILE *fd_common;
    int size;
    int size_common = 0;
    char *source;

    fd = fopen(source_path, "rb");
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    if (shader_type == GL_VERTEX_SHADER)
    {
        fd_common = fopen(_common, "rb");
        fseek(fd_common, 0, SEEK_END);
        size_common = ftell(fd_common);
        fseek(fd_common, 0, SEEK_SET);
    }

    source = malloc(size + size_common + 1);
    memset(source, 0x0, size + size_common + 1);
    if (shader_type == GL_VERTEX_SHADER)
    {
        fread(source, 1, size_common, fd_common);
        fclose(fd_common);
    }
    fread(source + size_common, 1, size, fd);
    fclose(fd);

    shader = glCreateShader(shader_type);
    if (!shader)
        exit(1);

    glShaderSource(shader, 1, (const char **) &source, NULL);
    glCompileShader(shader);

    free(source);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[1000];
        GLsizei len;
        glGetShaderInfoLog(shader, 1000, &len, log);
        fprintf(stderr, "Error: compiling %s: %.*s\n",
            shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
            len, log);
        exit(1);
    }

    return shader;
}

static lv_gl_prog_t * create_program(int vert_type, int frag_type)
{
    lv_gl_prog_t * prog;
    const char *vert_text;
    const char *frag_text;
    GLuint program;
    GLuint vert;
    GLuint frag;
    GLint status;

    switch (vert_type)
    {
    case VERTEX_DEFAULT:
        vert_text = _vert;
        break;
    case VERTEX_FOLD:
        vert_text = _vert_fold;
        break;
    default:
        printf("Invalid vert type\n");
        break;
    }
    switch(frag_type)
    {
    case IMAGESOURCE_BGRA:
        frag_text = _frag_bgra;
        break;
    case IMAGESOURCE_RGBA:
        frag_text = _frag_rgba;
        break;
    default:
        printf("Invalid frag type\n");
        break;
    }

    prog = malloc(sizeof(lv_gl_prog_t));

    frag = create_shader(frag_text, GL_FRAGMENT_SHADER);
    vert = create_shader(vert_text, GL_VERTEX_SHADER);

    program = glCreateProgram();
    glAttachShader(program, frag);
    glAttachShader(program, vert);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1000];
        GLsizei len;
        glGetProgramInfoLog(program, 1000, &len, log);
        fprintf(stderr, "Error: linking:\n%.*s\n", len, log);
        exit(1);
    }
    glUseProgram(program);

	glBindAttribLocation(program, gl_pos, "pos");
    glBindAttribLocation(program, gl_tex, "tex");
    glLinkProgram(program);

    glGenVertexArrays(1, &prog->VAO);
    glGenBuffers(1, &prog->VBO);
    glBindVertexArray(prog->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, prog->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts_2d),
        &verts_2d, GL_STATIC_DRAW);
    glEnableVertexAttribArray(gl_pos);
    glVertexAttribPointer(gl_pos, 4, GL_FLOAT,
        GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(gl_tex);
    glVertexAttribPointer(gl_tex, 2, GL_FLOAT,
        GL_FALSE, 6 * sizeof(GLfloat),
        (GLvoid *)(4 * sizeof(GLfloat)));

    glBindVertexArray(0);

    prog->program = program;
    prog->frag = frag;
    prog->vert = vert;

    return prog;
}

static lv_gl_prog_t * create_cube_program(int type)
{
    lv_gl_prog_t * prog;
    const char *vert_text;
    const char *frag_text;
    GLuint program;
    GLuint vert;
    GLuint frag;
    GLint status;

    switch(type)
    {
    case IMAGESOURCE_BGRA:
        vert_text = _vert_cube;
        frag_text = _frag_bgra_cube;
        break;
    case IMAGESOURCE_RGBA:
        vert_text = _vert_cube;
        frag_text = _frag_rgba_cube;
        break;
    }

    prog = malloc(sizeof(lv_gl_prog_t));

    frag = create_shader(frag_text, GL_FRAGMENT_SHADER);
    vert = create_shader(vert_text, GL_VERTEX_SHADER);

    program = glCreateProgram();
    glAttachShader(program, frag);
    glAttachShader(program, vert);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1000];
        GLsizei len;
        glGetProgramInfoLog(program, 1000, &len, log);
        fprintf(stderr, "Error: linking:\n%.*s\n", len, log);
        exit(1);
    }
    glUseProgram(program);

	glBindAttribLocation(program, gl_pos, "pos");
    glLinkProgram(program);

    glGenVertexArrays(1, &prog->VAO);
    glGenBuffers(1, &prog->VBO);
    glBindVertexArray(prog->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, prog->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts_cube),
        &verts_cube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(gl_pos);
    glVertexAttribPointer(gl_pos, 4, GL_FLOAT,
        GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glBindVertexArray(0);

    prog->program = program;
    prog->frag = frag;
    prog->vert = vert;

    return prog;
}

int lv_gl_ctx_init(void)
{
    printf("%s\n", glGetString(GL_RENDERER));
    printf("%s\n", glGetString(GL_VENDOR));
    printf("%s\n", glGetString(GL_VERSION));
    printf("%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    memset(&gl_ctx, 0x0, sizeof(gl_ctx));
    _lv_ll_init(&gl_ctx.base.ready_ll, sizeof(lv_gl_obj_t));
    _lv_ll_init(&gl_ctx.base.idle_ll,  sizeof(lv_gl_obj_t));

    gl_ctx.progs[GL_OBJ_TYPE_2D * 2 + 0] =
        create_program(VERTEX_DEFAULT, IMAGESOURCE_RGBA);
    gl_ctx.progs[GL_OBJ_TYPE_2D * 2 + 1] =
        create_program(VERTEX_DEFAULT, IMAGESOURCE_BGRA);
    gl_ctx.progs[GL_OBJ_TYPE_CUBE * 2 + 0] =
        create_cube_program(IMAGESOURCE_RGBA);
    gl_ctx.progs[GL_OBJ_TYPE_CUBE * 2 + 1] =
        create_cube_program(IMAGESOURCE_BGRA);
    gl_ctx.progs[GL_OBJ_TYPE_FOLD * 2 + 0] =
        create_program(VERTEX_FOLD, IMAGESOURCE_RGBA);
    gl_ctx.progs[GL_OBJ_TYPE_FOLD * 2 + 1] =
        create_program(VERTEX_FOLD, IMAGESOURCE_BGRA);

    gl_ctx.window = SDL_GL_GetCurrentWindow();
    gl_ctx.context = SDL_GL_GetCurrentContext();
    SDL_GetWindowSize(gl_ctx.window, &gl_ctx.base.w,
        &gl_ctx.base.h);

    gl_ctx.base.reverse_y = 1;
    gl_ctx.base.format = GL_OBJ_FMT_RGBA;
    reset_viewport(&gl_ctx.base);
    reset_crop(&gl_ctx.base);

    gl_ctx.inited = 1;
}

lv_gl_tex_t *lv_gl_tex_create(int type, int w, int h)
{
    SDL_GLContext previousContext;
    lv_gl_tex_t *new;
    int count = 1;

    if (!gl_ctx.inited) {
        printf("call lv_gl_ctx_init first\n");
        return NULL;
    }

    previousContext = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent(gl_ctx.window, gl_ctx.context);

    if (type == GL_OBJ_TYPE_FOLD)
        count = 2;

    new = lv_mem_alloc(sizeof(*new) * count);
    memset(new, 0, sizeof(*new) * count);
    for (int i = 0; i < count; i++)
    {
        glGenFramebuffers(1, &new[i].FBO);
        glGenTextures(1, &new[i].gl_tex);
    }
    if (type == GL_OBJ_TYPE_2D ||
        type == GL_OBJ_TYPE_FOLD)
    {
        for (int i = 0; i < count; i++)
        {
            glBindTexture(GL_TEXTURE_2D, new[i].gl_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
                0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    if (type == GL_OBJ_TYPE_CUBE)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, new->gl_tex);
        for (int i = CUBE_LEFT; i <= CUBE_BACK; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGBA, w, w,
                0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    SDL_GL_MakeCurrent(gl_ctx.window, previousContext);

    return new;
}

void lv_gl_tex_del(lv_gl_tex_t *tex)
{
    SDL_GLContext previousContext;

    previousContext = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent(gl_ctx.window, gl_ctx.context);

    glDeleteFramebuffers(1, &tex->FBO);
    glDeleteTextures(1, &tex->gl_tex);
    lv_mem_free(tex);

    SDL_GL_MakeCurrent(gl_ctx.window, previousContext);
}

lv_gl_obj_t *lv_gl_obj_create(lv_gl_obj_t *parent,
    int type, int w, int h)
{
    lv_gl_obj_t *new;
    lv_gl_base_t *base;
    lv_gl_base_t *pbase;

    if (!parent)
    {
        if (!gl_ctx.inited) {
            printf("call lv_gl_ctx_init first\n");
            return NULL;
        }
        pbase = &gl_ctx.base;
    }
    else
    {
        pbase = &parent->base;
    }

    new = _lv_ll_ins_tail(&pbase->idle_ll);
    if (!new)
        return NULL;
    memset(new, 0, sizeof(*new));
    new->alpha = 1.0;
    base = &new->base;
    _lv_ll_init(&base->ready_ll, sizeof(lv_gl_obj_t));
    _lv_ll_init(&base->idle_ll,  sizeof(lv_gl_obj_t));
    base->parent = pbase;
    base->ll = &pbase->idle_ll;
    base->type = type;
    base->format = GL_OBJ_FMT_BGRA;

    base->tex = lv_gl_tex_create(type, w, h);

    base->w = w;
    base->h = h;
    base->r.x = 0;
    base->r.y = 0;
    base->r.w = w;
    base->r.h = h;
    reset_crop(base);
    reset_viewport(base);

    return new;
}

void lv_gl_obj_del(lv_gl_obj_t *obj)
{
    _lv_ll_remove(obj->base.ll, obj);
    lv_gl_tex_del(obj->base.tex);
    lv_mem_free(obj);
}

static void import_img_cube(lv_gl_obj_t *obj,
    lv_gl_img_t *img)
{
    SDL_GLContext previousContext;
    lv_gl_base_t *pbase;

    for (int i = CUBE_LEFT; i <= CUBE_BACK; i++)
    {
        if (img[i].pixels &&
            (img[i].w != obj->base.w ||
             img[i].h != obj->base.w))
        {
            printf("[%d] error img size\n", i);
            return;
        }
    }

    previousContext = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent(gl_ctx.window, gl_ctx.context);

    pbase = obj->base.parent;
    glBindTexture(GL_TEXTURE_CUBE_MAP,
        obj->base.tex->gl_tex);
    for (int i = CUBE_LEFT; i <= CUBE_BACK; i++)
    {
        if (!img[i].pixels)
            continue;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGBA, obj->base.w, obj->base.w, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img[i].pixels);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    SDL_GL_MakeCurrent(gl_ctx.window, previousContext);
}

static void import_img_fold(lv_gl_obj_t *obj,
    lv_gl_img_t *img)
{
    SDL_GLContext previousContext;
    lv_gl_base_t *pbase;

    if (img[0].w != obj->base.w || img[0].h != obj->base.h)
    {
        printf("error img size\n");
        return;
    }
    if (img[1].w != obj->base.w || img[1].h != obj->base.h)
    {
        printf("error img size\n");
        return;
    }

    previousContext = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent(gl_ctx.window, gl_ctx.context);

    pbase = obj->base.parent;
    glBindTexture(GL_TEXTURE_2D, obj->base.tex[0].gl_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        obj->base.w, obj->base.h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, img[0].pixels);
    glBindTexture(GL_TEXTURE_2D, obj->base.tex[1].gl_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        obj->base.w, obj->base.h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, img[1].pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_GL_MakeCurrent(gl_ctx.window, previousContext);
}

static void import_img_2d(lv_gl_obj_t *obj,
    lv_gl_img_t *img)
{
    SDL_GLContext previousContext;
    lv_gl_base_t *pbase;

    if (img->w != obj->base.w || img->h != obj->base.h)
    {
        printf("error img size\n");
        return;
    }

    previousContext = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent(gl_ctx.window, gl_ctx.context);

    pbase = obj->base.parent;
    glBindTexture(GL_TEXTURE_2D, obj->base.tex->gl_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        obj->base.w, obj->base.h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, img->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_GL_MakeCurrent(gl_ctx.window, previousContext);
}

void lv_gl_obj_import_img(lv_gl_obj_t *obj, lv_gl_img_t *img)
{
    switch (obj->base.type)
    {
    case GL_OBJ_TYPE_2D:
        import_img_2d(obj, img);
        break;
    case GL_OBJ_TYPE_FOLD:
        import_img_fold(obj, img);
        break;
    case GL_OBJ_TYPE_CUBE:
        import_img_cube(obj, img);
        break;
    }
}

#define load_1f(x, f0) \
    do {               \
        uniform =      \
            glGetUniformLocation(prog->program, #x); \
        glUniform1f(uniform, f0);                    \
    } while (0);
#define load_vec3(x, f0, f1, f2) \
    do {                         \
        uniform =                \
            glGetUniformLocation(prog->program, #x); \
        glUniform3f(uniform, f0, f1, f2);            \
    } while (0);
static void render_fold(lv_gl_obj_t *obj)
{
    lv_gl_base_t *parent = obj->base.parent;
    lv_gl_prog_t *prog = gl_ctx.cur_prog;
    GLuint uniform;
    float scale;
    float scale_cx;
    float scale_cy;
    int x, y;
    int vp_w;
    int vp_h;
    int scr_w;
    int scr_h;
    uint32_t pos;
    float ofs_x;
    float ofs_y;
    float step;
    float ay = obj->ay;
    GLenum status;

    glUseProgram(prog->program);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    vp_w = parent->view.w;
    vp_h = parent->view.h;
    scr_w = parent->w;
    scr_h = parent->h;

    scale_cx = (float)obj->base.r.w / vp_w;
    scale_cy = (float)obj->base.r.h / vp_h;

    x = obj->base.r.x + (vp_w - scr_w) / 2;
    y = obj->base.r.y + (vp_h - scr_h) / 2;
    ofs_x = ((float)x / vp_w * 2.0 + (-1.0));
    ofs_y = 1 - ((float)y / vp_h * 2.0);

    step = 1.0 / obj->steps;
    pos = obj->pos;
    if (pos > obj->steps)
        pos = obj->steps;

    scale = pos * step;
    load_1f(alpha, obj->alpha);
    load_vec3(offset, 1.0, -1.0, 0.0);
    load_vec3(scale, scale * scale_cx, scale_cy, 1.0);
    load_vec3(rot, 0.0, 0.0, 0.0);
    load_vec3(move, ofs_x, ofs_y, 0.0);
    load_vec3(t_scale, scale, 1.0, 1.0);
    load_vec3(t_move, 0.0, 0.0, 0.0);

    glBindVertexArray(prog->VAO);

    glBindTexture(GL_TEXTURE_2D, obj->base.tex[0].gl_tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    ofs_x += 2.0 * scale * scale_cx;
    if ((pos < obj->steps) && ay < 180 && ay > 90)
    {
        load_vec3(offset, 1.0, -1.0, 0.0);
        load_vec3(scale, step * scale_cx, scale_cy, 1.0);
        load_vec3(rot, 0.0, ay - 180, 0.0);
        load_vec3(move, ofs_x, ofs_y, 0.0);
        load_vec3(t_scale, step, 1.0, 1.0);
        load_vec3(t_move, scale, 0.0, 0.0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glBindTexture(GL_TEXTURE_2D, obj->base.tex[1].gl_tex);
    if ((pos < obj->steps) && ay > 0.0 && ay < 90)
    {
        load_vec3(offset, -1.0, -1.0, 0.0);
        load_vec3(scale, step * scale_cx, scale_cy, 1.0);
        load_vec3(rot, 0.0, ay, 0.0);
        load_vec3(move, ofs_x, ofs_y, 0.0);
        load_vec3(t_scale, step, 1.0, 1.0);
        load_vec3(t_move, (pos - 1) * step, 0.0, 0.0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    load_vec3(offset, 1.0, -1.0, 0.0);
    load_vec3(scale, (1 - scale) * scale_cx, scale_cy, 1.0);
    load_vec3(rot, 0.0, 0.0, 0.0);
    load_vec3(move, ofs_x, ofs_y, 0.0);
    load_vec3(t_scale, (1 - scale), 1.0, 1.0);
    load_vec3(t_move, scale, 0.0, 0.0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);
}

static void render_cube(lv_gl_obj_t *obj)
{
    lv_gl_prog_t *prog = gl_ctx.cur_prog;
    GLfloat scale;
    GLuint uniform;
    GLenum status;

    glUseProgram(prog->program);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    scale = obj->scale;

    load_vec3(scale, scale, scale, scale);

    load_vec3(rot, obj->ax, obj->ay, obj->az);
    load_vec3(vrot, obj->view_ax, obj->view_ay,
        obj->view_az);
    load_vec3(move, 0.0, 0.0, 0.0);

    glBindVertexArray(prog->VAO);

    glBindTexture(GL_TEXTURE_CUBE_MAP, obj->base.tex->gl_tex);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glBindVertexArray(0);
}

static void render_2d(lv_gl_obj_t *obj)
{
    lv_gl_base_t *parent = obj->base.parent;
    lv_gl_prog_t *prog = gl_ctx.cur_prog;
    GLuint uniform;
    GLfloat ofs_x, ofs_y;
    GLuint x, y;
    GLuint w, h;
    GLenum status;

    glUseProgram(prog->program);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    w = parent->view.w;
    h = parent->view.h;

    x = obj->base.r.x + (w - parent->w) / 2;
    y = obj->base.r.y + (h - parent->h) / 2;
    ofs_x = ((float)x / w * 2.0 + (-1.0)) +
        (float)obj->base.r.w / w;
    ofs_y = (((float)y / h * 2.0 + (-1.0)) +
        (float)obj->base.r.h / h);
    if (parent->reverse_y)
        ofs_y = -ofs_y;

    load_1f(alpha, obj->alpha);
    load_vec3(offset, 0.0, 0.0, obj->offset_z);
    load_vec3(scale, (float)obj->base.r.w / w, (float)obj->base.r.h / h, 1.0);
    load_vec3(rot, obj->ax, obj->ay, obj->az);
    load_vec3(vrot, obj->view_ax, obj->view_ay, obj->view_az);
    load_vec3(move, ofs_x, ofs_y, 0.0);

    glBindVertexArray(prog->VAO);

    glBindTexture(GL_TEXTURE_2D, obj->base.tex->gl_tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
}
#undef load_vec3
#undef load_1f

static void reset_crop(lv_gl_base_t *base)
{
    base->crop.x = 0;
    base->crop.y = 0;
    base->crop.w = base->w;
    base->crop.h = base->h;
    base->crop_dirty = 1;
}

static void reset_viewport(lv_gl_base_t *base)
{
    if (base->w > base->h)
    {
        base->view.x = 0;
        base->view.y =
            (base->h - base->w) / 2;
        base->view.w = base->w;
        base->view.h = base->w;
    }
    else if (base->w < base->h)
    {
        base->view.x =
            (base->w - base->h) / 2;
        base->view.y = 0;
        base->view.w = base->h;
        base->view.h = base->h;
    }
    else
    {
        base->view.x = 0;
        base->view.y = 0;
        base->view.w = base->w;
        base->view.h = base->h;
    }
    base->view_dirty = 1;
}

static void update_crop_viewport(lv_gl_base_t *base)
{
    if (1 || base->crop_dirty)
    {
        if (base->crop_en)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(base->crop.x, base->crop.y,
                base->crop.w, base->crop.h);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }
        base->crop_dirty = 0;
    }

    if (1 || base->view_dirty)
    {
        glViewport(base->view.x, base->view.y,
            base->view.w, base->view.h);
        base->view_dirty = 0;
    }
}

static void render(lv_gl_obj_t *obj)
{
    lv_gl_base_t *parent;
    GLuint swap;
    GLenum status;

    if (!obj || obj->base.type > GL_OBJ_TYPE_FOLD)
        return;

    parent = (lv_gl_base_t *)obj->base.parent;
    if (!parent) parent = &gl_ctx.base;
    if (parent->tex == NULL)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, parent->tex->FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, obj->out_type,
            parent->tex->gl_tex, 0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("bind framebuffer error\n");
            return;
        }
    }
    update_crop_viewport(parent);

    swap = (obj->base.format != parent->format);

    gl_ctx.cur_prog =
        gl_ctx.progs[obj->base.type * 2 + swap];

    switch (obj->base.type)
    {
    case GL_OBJ_TYPE_2D:
        render_2d(obj);
        break;
    case GL_OBJ_TYPE_FOLD:
        render_fold(obj);
        break;
    case GL_OBJ_TYPE_CUBE:
        render_cube(obj);
        break;
    }

}

void lv_gl_obj_ready(lv_gl_obj_t *obj, int always)
{
    lv_gl_base_t *parent = obj->base.parent;
    obj->always_ready = always;

    if (obj->base.ll == &parent->ready_ll)
        return;

    _lv_ll_chg_list(obj->base.ll, &parent->ready_ll, obj, 0);
    obj->base.ll = &parent->ready_ll;
}

void lv_gl_obj_idle(lv_gl_obj_t *obj)
{
    lv_gl_base_t *parent = obj->base.parent;

    if (obj->base.ll == &parent->idle_ll)
        return;

    _lv_ll_chg_list(obj->base.ll, &parent->idle_ll, obj, 0);
    obj->base.ll = &parent->idle_ll;
}

void lv_gl_set_viewport(lv_gl_obj_t *obj, SDL_Rect *r)
{
    lv_gl_base_t *base;

    if (!obj)
    {
        if (!gl_ctx.inited) {
            printf("call lv_gl_ctx_init first\n");
            return;
        }
        base = &gl_ctx.base;
    }
    else
    {
        base = &obj->base;
    }

    if (r)
    {
        base->view.x = r->x;
        base->view.y = r->y;
        base->view.w = r->w;
        base->view.h = r->h;
    }
    else
    {
        reset_viewport(base);
    }
    base->view_dirty = 1;
}

void lv_gl_get_viewport(lv_gl_obj_t *obj, SDL_Rect *r)
{
    lv_gl_base_t *base;

    if (!obj)
    {
        if (!gl_ctx.inited) {
            printf("call lv_gl_ctx_init first\n");
            return;
        }
        base = &gl_ctx.base;
    }
    else
    {
        base = &obj->base;
    }

    if (r)
    {
        r->x = base->view.x;
        r->y = base->view.y;
        r->w = base->view.w;
        r->h = base->view.h;
    }
}

void lv_gl_set_crop(lv_gl_obj_t *obj, SDL_Rect *r, int en)
{
    lv_gl_base_t *base;

    if (!obj)
    {
        if (!gl_ctx.inited) {
            printf("call lv_gl_ctx_init first\n");
            return;
        }
        base = &gl_ctx.base;
    }
    else
    {
        base = &obj->base;
    }

    if (r)
    {
        base->crop.x = r->x;
        base->crop.y = r->y;
        base->crop.w = r->w;
        base->crop.h = r->h;
    }
    else
    {
        reset_crop(base);
    }
    base->crop_en = en;
    base->crop_dirty = 1;
}

void lv_gl_get_crop(lv_gl_obj_t *obj, SDL_Rect *r)
{
    lv_gl_base_t *base;

    if (!obj)
    {
        if (!gl_ctx.inited) {
            printf("call lv_gl_ctx_init first\n");
            return;
        }
        base = &gl_ctx.base;
    }
    else
    {
        base = &obj->base;
    }

    if (r)
    {
        r->x = base->crop.x;
        r->y = base->crop.y;
        r->w = base->crop.w;
        r->h = base->crop.h;
    }
}

static void render_list(lv_gl_base_t *base)
{
    lv_gl_obj_t *obj;
    lv_ll_t *ready_ll = &base->ready_ll;
    lv_ll_t *idle_ll  = &base->idle_ll;

    _LV_LL_READ(ready_ll, obj)
    {
        render_list(&obj->base);
        render(obj);
        if (!obj->always_ready)
        {
            _lv_ll_chg_list(obj->base.ll, idle_ll, obj, 0);
            obj->base.ll = idle_ll;
        }
    }
}

void lv_gl_render(void)
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_ZERO,
        GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    render_list(&gl_ctx.base);
}
#endif

