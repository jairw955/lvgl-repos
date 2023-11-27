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

#define LOGD            //printf

#define DATA_PATH       "/usr/share/lv_drivers/shaders"

static char *_common = DATA_PATH"/common.vert";
static char *_vert = DATA_PATH"/default.vert";
static char *_vert_cube = DATA_PATH"/cube.vert";
static char *_frag_bgra = DATA_PATH"/bgra.frag";
static char *_frag_rgba = DATA_PATH"/rgba.frag";
static char *_frag_bgra_cube = DATA_PATH"/bgra_cube.frag";
static char *_frag_rgba_cube = DATA_PATH"/rgba_cube.frag";

static const lv_gl_vec_t default_vec[4] = {
    /* bottom left */
    {-1.0, -1.0, 0.0},
    /* bottom right */
    { 1.0, -1.0, 0.0},
    /* top left */
    {-1.0,  1.0, 0.0},
    /* top right */
    { 1.0,  1.0, 0.0},
};

static const GLfloat verts_2d[] = {
    /* bottom left */
    -1.0, -1.0, 0.0, 1.0, 0.0, 1.0,
    /* bottom right */
     1.0, -1.0, 0.0, 1.0, 1.0, 1.0,
    /* top left */
    -1.0,  1.0, 0.0, 1.0, 0.0, 0.0,
    /* top right */
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
    lv_gl_base_t * fb;
    int inited;
    int clear_depth;
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

static lv_gl_render_cb_t user_cb = NULL;

void lv_gl_set_render_cb(lv_gl_render_cb_t cb)
{
    user_cb = cb;
}

static inline lv_gl_tex_t *tex_ref(lv_gl_tex_t *tex)
{
    tex->ref_cnt++;

    return tex;
}

static inline lv_gl_tex_t *tex_unref(lv_gl_tex_t *tex)
{
    if ((--tex->ref_cnt) > 0)
        return tex;

    lv_gl_tex_del(tex);

    return NULL;
}

static SDL_GLContext enter_critical(void)
{
    SDL_GLContext ctx;

    ctx = SDL_GL_GetCurrentContext();
    if (ctx != gl_ctx.context)
        SDL_GL_MakeCurrent(gl_ctx.window, gl_ctx.context);
}

static void exit_critical(SDL_GLContext ctx)
{
    if (ctx != gl_ctx.context)
        SDL_GL_MakeCurrent(gl_ctx.window, ctx);
}

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

    gl_ctx.progs[GL_TEX_TYPE_2D * 2 + 0] =
        create_program(VERTEX_DEFAULT, IMAGESOURCE_RGBA);
    gl_ctx.progs[GL_TEX_TYPE_2D * 2 + 1] =
        create_program(VERTEX_DEFAULT, IMAGESOURCE_BGRA);
    gl_ctx.progs[GL_TEX_TYPE_CUBE * 2 + 0] =
        create_cube_program(IMAGESOURCE_RGBA);
    gl_ctx.progs[GL_TEX_TYPE_CUBE * 2 + 1] =
        create_cube_program(IMAGESOURCE_BGRA);

    gl_ctx.window = SDL_GL_GetCurrentWindow();
    gl_ctx.context = SDL_GL_GetCurrentContext();
    SDL_GetWindowSize(gl_ctx.window, &gl_ctx.base.w,
        &gl_ctx.base.h);

    gl_ctx.base.reverse_y = 1;
    reset_viewport(&gl_ctx.base);
    reset_crop(&gl_ctx.base);

    gl_ctx.fb = &gl_ctx.base;
    gl_ctx.inited = 1;
}

void lv_gl_set_fb(lv_gl_obj_t *obj)
{
    lv_gl_base_t *fb;

    if (obj)
        fb = &obj->base;
    else
        fb = &gl_ctx.base;

    if (gl_ctx.fb == fb)
        return;

    gl_ctx.fb = fb;
    gl_ctx.clear_depth = 1;
}

void lv_gl_read_pixels(void *ptr, SDL_Rect *r, int type)
{
    SDL_GLContext previousContext;
    GLenum status;
    lv_gl_base_t *fb;

    previousContext = enter_critical();
    fb = gl_ctx.fb;
    if (fb->tex == NULL)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fb->tex->FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            type ? type : GL_TEXTURE_2D, fb->tex->gl_tex, 0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("bind framebuffer error\n");
            return;
        }
    }

    glReadPixels(r->x, r->y, r->w, r->h, GL_RGBA, GL_UNSIGNED_BYTE, ptr);

    exit_critical(previousContext);
}

lv_gl_tex_t *lv_gl_tex_create(int type, int w, int h, lv_gl_img_t *img)
{
    SDL_GLContext previousContext;
    lv_gl_tex_t *new;
    int count = 1;

    if (!gl_ctx.inited) {
        printf("call lv_gl_ctx_init first\n");
        return NULL;
    }

    previousContext = enter_critical();

    new = lv_mem_alloc(sizeof(*new));
    memset(new, 0, sizeof(*new));
    new->type = type;
    glGenFramebuffers(1, &new->FBO);
    glGenTextures(1, &new->gl_tex);

    if (type == GL_TEX_TYPE_2D)
    {
        glBindTexture(GL_TEXTURE_2D, new->gl_tex);
        if (img && img->pixels)
        {
            new->size[0].w = img->w;
            new->size[0].h = img->h;
            new->format = img->format;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h,
                0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
        }
        else
        {
            new->size[0].w = w;
            new->size[0].h = h;
            new->format = LV_GL_FMT_BGRA;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h,
                0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (type == GL_TEX_TYPE_CUBE)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, new->gl_tex);
        for (int i = CUBE_LEFT; i <= CUBE_BACK; i++)
        {
            if (img && img[i].pixels)
            {
                new->size[0].w = img[i].w;
                new->size[0].h = img[i].h;
                new->format = img->format;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGBA, img[i].w, img[i].h,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, img[i].pixels);
            }
            else
            {
                new->size[0].w = w;
                new->size[0].h = h;
                new->format = LV_GL_FMT_RGBA;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGBA, w, h,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            }
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

    exit_critical(previousContext);

    return new;
}

void lv_gl_tex_del(lv_gl_tex_t *tex)
{
    SDL_GLContext previousContext;

    previousContext = enter_critical();

    glDeleteFramebuffers(1, &tex->FBO);
    glDeleteTextures(1, &tex->gl_tex);
    lv_mem_free(tex);

    exit_critical(previousContext);
}

static void import_img_cube(lv_gl_tex_t *tex,
    lv_gl_img_t *img)
{
    SDL_GLContext previousContext;

    previousContext = enter_critical();

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->gl_tex);
    for (int i = CUBE_LEFT; i <= CUBE_BACK; i++)
    {
        if (!img[i].pixels)
            continue;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGBA, img[i].w, img[i].h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            img[i].pixels);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    tex->format = img->format;

    exit_critical(previousContext);
}

static void import_img_2d(lv_gl_tex_t *tex, lv_gl_img_t *img)
{
    SDL_GLContext previousContext;

    previousContext = enter_critical();

    glBindTexture(GL_TEXTURE_2D, tex->gl_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        img->w, img->h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, img->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    tex->format = img->format;

    exit_critical(previousContext);
}

void lv_gl_tex_import_img(lv_gl_tex_t *tex, lv_gl_img_t *img)
{
    switch (tex->type)
    {
    case GL_TEX_TYPE_2D:
        import_img_2d(tex, img);
        break;
    case GL_TEX_TYPE_CUBE:
        import_img_cube(tex, img);
        break;
    }
}

void lv_gl_tex_clear(lv_gl_tex_t *tex, float r, float g, float b, float a)
{
    SDL_GLContext previousContext;
    GLenum status;

    previousContext = enter_critical();

    glBindFramebuffer(GL_FRAMEBUFFER, tex->FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, tex->gl_tex, 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("bind framebuffer error\n");
        exit_critical(previousContext);
        return;
    }

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    exit_critical(previousContext);
}

lv_gl_obj_t *lv_gl_obj_create(int w, int h)
{
    lv_gl_obj_t *new;

    new = lv_mem_alloc(sizeof(*new));
    if (!new)
        return NULL;
    memset(new, 0, sizeof(*new));
    new->base.w = w;
    new->base.h = h;
    new->alpha = 1.0;
    new->scale.x = 1.0;
    new->scale.y = 1.0;
    new->scale.z = 1.0;
    new->tp.w = w;
    new->tp.h = h;
    reset_crop(&new->base);
    reset_viewport(&new->base);
    lv_gl_obj_reset_points(new);

    return new;
}

void lv_gl_obj_del(lv_gl_obj_t *obj)
{
    lv_gl_tex_t *tex;

    tex = obj->base.tex;
    if (tex)
        tex_unref(tex);
    lv_mem_free(obj);
}

void lv_gl_obj_resize(lv_gl_obj_t *obj, lv_gl_obj_t *parent)
{
    SDL_Rect r;

    lv_gl_obj_get_viewport(parent, &r);

    obj->scale.x = (float)obj->base.w / r.w;
    obj->scale.y = (float)obj->base.h / r.h;
    obj->scale.z = 1.0;
}

void lv_gl_obj_move(lv_gl_obj_t *obj, lv_gl_obj_t *parent)
{
    SDL_Rect r;

    lv_gl_obj_get_viewport(parent, &r);

    obj->move.x = obj->scale.x + ((float)obj->base.r.x / r.w * 2.0 - 1.0);
    obj->move.y = obj->scale.y + ((float)obj->base.r.y / r.h * 2.0 - 1.0);
}

void lv_gl_obj_bind_tex(lv_gl_obj_t *obj, lv_gl_tex_t *tex)
{
    if (obj->base.tex)
        tex_unref(obj->base.tex);

    obj->base.tex = tex_ref(tex);
    lv_gl_obj_reset_tex_points(obj);
}

void lv_gl_obj_update_vao(lv_gl_obj_t *obj)
{
    SDL_GLContext previousContext;
    SDL_Rect *r;
    lv_gl_tex_t *tex;
    lv_gl_base_t *fb = gl_ctx.fb;
    GLuint swap;
    int format;
    GLfloat x1, x2, y1, y2;
    GLfloat verts[] = {
        -1.0, -1.0, 0.0, 1.0, 0.0, 1.0,
         1.0, -1.0, 0.0, 1.0, 1.0, 1.0,
        -1.0,  1.0, 0.0, 1.0, 0.0, 0.0,
         1.0,  1.0, 0.0, 1.0, 1.0, 0.0,
    };

    if (!obj || !obj->base.tex)
        return;
    tex = obj->base.tex;

    previousContext = enter_critical();

    if (fb->tex == NULL)
        format = LV_GL_FMT_RGBA;
    else
        format = fb->tex->format;

    if (!obj->VAO)
    {
        glGenVertexArrays(1, &obj->VAO);
        glGenBuffers(1, &obj->VBO);
    }

    r  = &obj->tp;
    x1 = (float)r->x / lv_gl_tex_get_w(tex, 0);
    x2 = (float)(r->x + r->w) / lv_gl_tex_get_w(tex, 0);
    y1 = (float)r->y / lv_gl_tex_get_h(tex, 0);
    y2 = (float)(r->y + r->h) / lv_gl_tex_get_h(tex, 0);

    verts[6 * 0 + 4] = x1;
    verts[6 * 0 + 5] = y2;
    verts[6 * 1 + 4] = x2;
    verts[6 * 1 + 5] = y2;
    verts[6 * 2 + 4] = x1;
    verts[6 * 2 + 5] = y1;
    verts[6 * 3 + 4] = x2;
    verts[6 * 3 + 5] = y1;

    memcpy(&verts[0],  &obj->p[0], sizeof(lv_gl_vec_t));
    memcpy(&verts[6],  &obj->p[1], sizeof(lv_gl_vec_t));
    memcpy(&verts[12], &obj->p[2], sizeof(lv_gl_vec_t));
    memcpy(&verts[18], &obj->p[3], sizeof(lv_gl_vec_t));

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            LOGD("%f ", verts[i * 6 + j]);
        }
        LOGD("\n");
    }
    LOGD("\n");

    swap = (obj->base.tex->format != format);
    glUseProgram(gl_ctx.progs[tex->type * 2 + swap]->program);
    glBindVertexArray(obj->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(gl_pos);
    glVertexAttribPointer(gl_pos, 4, GL_FLOAT,
        GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(gl_tex);
    glVertexAttribPointer(gl_tex, 2, GL_FLOAT,
        GL_FALSE, 6 * sizeof(GLfloat),
        (GLvoid *)(4 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glUseProgram(0);

    exit_critical(previousContext);
}

void lv_gl_obj_release_vao(lv_gl_obj_t *obj)
{
    SDL_GLContext previousContext;

    if (!obj || !obj->VAO)
        return;

    previousContext = enter_critical();

    glDeleteVertexArrays(1, &obj->VAO);
    glDeleteBuffers(1, &obj->VBO);
    obj->VAO = 0;
    obj->VBO = 0;

    exit_critical(previousContext);
}

void lv_gl_obj_reset_points(lv_gl_obj_t *obj)
{
    memcpy(obj->p, default_vec, sizeof(default_vec));
}

void lv_gl_obj_reset_tex_points(lv_gl_obj_t *obj)
{
    int w, h;

    if (!obj->base.tex)
        return;

    w = lv_gl_tex_get_w(obj->base.tex, 0);
    h = lv_gl_tex_get_h(obj->base.tex, 0);

    obj->tp.x = 0;
    obj->tp.y = 0;
    obj->tp.w = w;
    obj->tp.h = h;
}

#define load_1f(x, f0) \
    do {               \
        LOGD(#x" "#f0"=%f\n", f0); \
        uniform =      \
            glGetUniformLocation(program, #x);  \
        glUniform1f(uniform, f0);               \
    } while (0);
#define load_vec3(x, f0, f1, f2) \
    do {                         \
        LOGD(#x" "#f0"=%f "#f1"=%f "#f2"=%f\n", f0, f1, f2); \
        uniform =                \
            glGetUniformLocation(program, #x);  \
        glUniform3f(uniform, f0, f1, f2);       \
    } while (0);

static void render_cube(lv_gl_obj_t *obj, GLuint program)
{
    lv_gl_tex_t *tex;
    GLuint uniform;

    tex = obj->base.tex;

    load_vec3(scale, obj->scale.x, obj->scale.y, obj->scale.z);
    load_vec3(rot, obj->self_rot.x, obj->self_rot.y, obj->self_rot.z);
    load_vec3(vrot, obj->view_rot.x, obj->view_rot.y, obj->view_rot.z);
    load_vec3(move, obj->move.x, obj->move.y, obj->move.z);

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->gl_tex);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static void render_2d(lv_gl_obj_t *obj, GLuint program)
{
    lv_gl_tex_t *tex;
    GLuint uniform;

    tex = obj->base.tex;

    load_1f(alpha, obj->alpha);
    load_vec3(offset, obj->offset.x, obj->offset.y, obj->offset.z);
    load_vec3(scale, obj->scale.x, obj->scale.y, obj->scale.z);
    load_vec3(rot, obj->self_rot.x, obj->self_rot.y, obj->self_rot.z);
    load_vec3(vrot, obj->view_rot.x, obj->view_rot.y, obj->view_rot.z);
    load_vec3(move, obj->move.x, obj->move.y, obj->move.z);

    glBindTexture(GL_TEXTURE_2D, tex->gl_tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

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
    base->view.x = 0;
    base->view.y = 0;
    base->view.w = base->w;
    base->view.h = base->h;
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
        LOGD("%s %d %d %d %d\n", __func__, base->view.x, base->view.y,
            base->view.w, base->view.h);
        glViewport(base->view.x, base->view.y,
            base->view.w, base->view.h);
        base->view_dirty = 0;
    }
}

static void render(lv_gl_obj_t *obj)
{
    lv_gl_tex_t *tex;
    lv_gl_base_t *fb;
    lv_gl_prog_t *prog;
    GLuint swap;
    GLenum status;
    int format;

    if (!obj || !obj->base.tex)
        return;
    tex = obj->base.tex;

    fb = gl_ctx.fb;
    if (fb->tex == NULL)
    {
        format = LV_GL_FMT_RGBA;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        format = fb->tex->format;
        glBindFramebuffer(GL_FRAMEBUFFER, fb->tex->FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            obj->out_type ? obj->out_type : GL_TEXTURE_2D,
            fb->tex->gl_tex, 0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("bind framebuffer error\n");
            return;
        }
    }
    update_crop_viewport(fb);

    swap = (obj->base.tex->format != format);
    prog = gl_ctx.progs[tex->type * 2 + swap];

    glUseProgram(prog->program);

    if (obj->VAO)
    {
        LOGD("Use obj->VAO\n");
        glBindVertexArray(obj->VAO);
    }
    else
    {
        LOGD("Use prog->VAO\n");
        glBindVertexArray(prog->VAO);
    }

    if (gl_ctx.clear_depth)
    {
        glEnable(GL_DEPTH_TEST);
        glClearDepthf(0.0);
        glDepthFunc(GL_GREATER);
        glClear(GL_DEPTH_BUFFER_BIT);
        gl_ctx.clear_depth = 0;
    }

    if (tex->type == GL_TEX_TYPE_2D)
        render_2d(obj, prog->program);
    else
        render_cube(obj, prog->program);

    glBindVertexArray(0);
}

void lv_gl_obj_render(lv_gl_obj_t *obj)
{
    SDL_GLContext previousContext;
    previousContext = enter_critical();
    render(obj);
    exit_critical(previousContext);
}

void lv_gl_obj_set_viewport(lv_gl_obj_t *obj, SDL_Rect *r)
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

void lv_gl_obj_get_viewport(lv_gl_obj_t *obj, SDL_Rect *r)
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

void lv_gl_obj_set_crop(lv_gl_obj_t *obj, SDL_Rect *r, int en)
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

void lv_gl_obj_get_crop(lv_gl_obj_t *obj, SDL_Rect *r)
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

void lv_gl_render(void)
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
        GL_ZERO, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    gl_ctx.clear_depth = 1;
    if (user_cb)
        user_cb();
    glFinish();
}
#endif

