#ifndef __DRM_DISPLAY_H__
#define __DRM_DISPLAY_H__

struct drm_bo {
    int fd;
    void *ptr;
    size_t size;
    size_t offset;
    size_t pitch;
    unsigned int handle;
    int fb_id;
    int buf_fd;
    int w;
    int h;
};

int drm_init(int bpp);
int drm_deinit(void);
void getdrmdispbuff(char **buff);
void getdrmdispfd(int *fd);
int getdrmdispinfo(struct drm_bo *bo, int *w, int *h);
struct drm_bo *getdrmdisp(void);
void setdrmdisp(struct drm_bo *bo);
int drm_setmode(int num, int bpp);
int drm_invalide(void);
void getdrmdispbpp(int *bpp);
void getdrmresolve(int *w, int *h);
void getdrmvideodispbuff(char **buff);
void getdrmvideodispfd(int *fd);
int getdrmvideodispinfo(struct drm_bo *bo, int *w, int *h);
struct drm_bo *getdrmvideodisp(void);
void setdrmvideodisp(struct drm_bo *bo);
void drmvideodispblack(void);
int getdrmwstride(void);
int getdrmvideowstride(void);
int getdrmrot(void);
int getdrmfd(void);

int drm_screen_off(void);
int drm_screen_on(void);

int createDrmVideoDisp(int format, int w, int h);
int destroyDrmVideoDisp(void);

struct drm_bo * malloc_drm_bo(int width, int height, int format);
void free_drm_bo(struct drm_bo *bo);

#endif
