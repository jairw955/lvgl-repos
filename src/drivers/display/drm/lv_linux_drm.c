/**
 * @file lv_linux_drm.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_linux_drm.h"

#if LV_USE_LINUX_DRM

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <poll.h>
#include <malloc.h>

#include <drm_fourcc.h>
#include <drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "../lv_display_common.h"

#define NUM_DUMB_BO 3
#define ALIGN(x, a)     (((x) + (a - 1)) & ~(a - 1))

#if (LV_COLOR_DEPTH == 16)
#define DRM_FORMAT      DRM_FORMAT_RGB565
#define RGA_FORMAT      RK_FORMAT_BGR_565
#define SW_ROTATION     1
#elif (LV_COLOR_DEPTH == 24)
#define DRM_FORMAT      DRM_FORMAT_RGB888
#define RGA_FORMAT      RK_FORMAT_BGR_888
#define SW_ROTATION     0
#elif (LV_COLOR_DEPTH == 32)
#define DRM_FORMAT      DRM_FORMAT_ARGB8888
#define RGA_FORMAT      RK_FORMAT_BGRA_8888
#define SW_ROTATION     1
#else
    #error "Unsupported depth"
#endif

#ifndef LV_DRM_USE_RGA
#define LV_DRM_USE_RGA 0
#endif

#if LV_DRM_USE_RGA
#undef SW_ROTATION
#define SW_ROTATION     0
#include <rga/im2d.h>
#include <rga/rga.h>
#include <rga/RgaApi.h>
#endif

typedef struct drm_bo {
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
} drm_bo_t;

typedef struct drm_argument {
    char *key;
    char *value;
} drm_argv_t;

typedef struct drm_device {
    int fd;
    lv_ll_t arguments;
    char *config;

    int preferred_width;
    int preferred_height;
    struct {
        int width;
        int height;

        int physical_width;
        int physical_height;
    } mode;

    drmModeResPtr res;

    int connector_id;
    int encoder_id;
    int crtc_id;
    int plane_id;
    bool is_primary_plane;
    int vsync;
	drmModeModeInfo mode_info;

    int waiting_for_flip;
    struct pollfd drm_pollfd;
    drmEventContext drm_evctx;

    int flush_buf_stride;
    char * flush_buf;
    lv_color_t * disp_buf;
    int disp_rot;
    int disp_crop;

    int quit;
    pthread_t pid;
    pthread_mutex_t mutex;
    int draw_update;

    drm_bo_t *vop_buf[2];
#if LV_DRM_USE_RGA
    drm_bo_t *gbo;
#endif
    overlay_dma_buffer_t overlay;
} drm_device_t;

static int bo_map(drm_device_t *dev, drm_bo_t *bo)
{
    struct drm_mode_map_dumb arg = {
        .handle = bo->handle,
    };
    struct drm_prime_handle fd_args = {
        .fd = -1,
        .handle = bo->handle,
        .flags = 0,
    };
    int ret;

    ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
    if (ret)
        return ret;

    ret = drmIoctl(dev->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &fd_args);
    if (ret)
    {
        LV_LOG_ERROR("handle_to_fd failed ret=%d, handle=%x", ret ,fd_args.handle);
        return -1;
    }
    bo->buf_fd = fd_args.fd;

    bo->ptr = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                   dev->fd, arg.offset);
    if (bo->ptr == MAP_FAILED) {
        bo->ptr = NULL;
        return -1;
    }

    return 0;
}

static void bo_unmap(drm_device_t *dev, drm_bo_t *bo)
{
    if (dev == NULL)
        return;
    if (!bo->ptr)
        return;

    drmUnmap(bo->ptr, bo->size);
    if (bo->buf_fd > 0)
        close(bo->buf_fd);
    bo->ptr = NULL;
}

void bo_destroy(drm_device_t *dev, drm_bo_t *bo)
{
    struct drm_mode_destroy_dumb arg = {
        .handle = bo->handle,
    };

    if (bo->fb_id)
        drmModeRmFB(dev->fd, bo->fb_id);

    bo_unmap(dev, bo);

    if (bo->handle)
        drmIoctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);

    free(bo);
}

static drm_bo_t *
bo_create(drm_device_t *dev, int width, int height, int format)
{
    struct drm_mode_create_dumb arg = {
        .bpp = LV_COLOR_DEPTH,
        .width = ALIGN(width, 16),
        .height = ALIGN(height, 16),
    };
    drm_bo_t *bo;
    uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
    int ret;

    bo = malloc(sizeof(drm_bo_t));
    if (bo == NULL) {
        LV_LOG_ERROR("allocate bo failed");
        return NULL;
    }
    memset(bo, 0, sizeof(*bo));
    if (format == DRM_FORMAT_NV12) {
        arg.bpp = 8;
        arg.height = height * 3 / 2;
    }

    ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
    if (ret) {
        LV_LOG_ERROR("create dumb failed");
        goto err;
    }

    bo->fd = dev->fd;
    bo->handle = arg.handle;
    bo->size = arg.size;
    bo->pitch = arg.pitch;
    bo->w = width;
    bo->h = height;

    ret = bo_map(dev, bo);
    if (ret) {
        LV_LOG_ERROR("map bo failed");
        goto err;
    }

    switch (format) {
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV16:
      handles[0] = bo->handle;
      pitches[0] = bo->pitch ;
      offsets[0] = 0;
      handles[1] = bo->handle;
      pitches[1] = pitches[0];
      offsets[1] = pitches[0] * height;
      break;
    case DRM_FORMAT_RGB332:
      handles[0] = bo->handle;
      pitches[0] = bo->pitch;
      offsets[0] = 0;
      break;
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
      handles[0] = bo->handle;
      pitches[0] = bo->pitch ;
      offsets[0] = 0;
      break;
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
      handles[0] = bo->handle;
      pitches[0] = bo->pitch ;
      offsets[0] = 0;
      break;
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_BGRA8888:
      handles[0] = bo->handle;
      pitches[0] = bo->pitch ;
      offsets[0] = 0;
      break;
    }

    ret = drmModeAddFB2(dev->fd, width, height, format, handles,
                        pitches, offsets, (uint32_t *)&bo->fb_id, 0);
    if (ret) {
        LV_LOG_ERROR("add fb failed");
        goto err;
    }
    LV_LOG_TRACE("Created bo: %d, %dx%d", bo->fb_id, width, height);

    return bo;
err:
    bo_destroy(dev, bo);
    return NULL;
}

drm_bo_t *malloc_drm_bo(drm_device_t *dev, int width, int height, int format)
{
    return bo_create(dev, width, height, format);
}

void free_drm_bo(drm_device_t *dev, drm_bo_t *bo)
{
    if (bo)
        bo_destroy(dev, bo);
}

static bool config_key_character_invalid(char c)
{
    if (c == '_') return false;
    if (c >= '0' && c <= '9') return false;
    if (c >= 'A' && c <= 'Z') return false;
    if (c >= 'a' && c <= 'z') return false;

    return true;
}

static bool config_value_character_invalid(char c)
{
    if (c == ';') return true;

    /* printable character except space */
    if (c > 0x20 && c <= 0x7e) return false;

    return true;
}

static char *drm_config_find_value(drm_device_t *dev, const char *key)
{
    drm_argv_t *argv = _lv_ll_get_head(&dev->arguments);
    while(argv) {
        if (strcmp(argv->key, key) == 0) return argv->value;
        argv = _lv_ll_get_next(&dev->arguments, argv);
    }

    return NULL;
}

static int drm_config_get_int_property(drm_device_t *dev, const char *name, int default_value)
{
    char *env = getenv(name);
    char *config = NULL;

    /* Prefer using env variable */
    if (env) {
        if ((env[0] >= '0') && (env[0] <= '9'))
            return atoi(env);
    }

    if (dev->config)
        config = drm_config_find_value(dev, name);
    if (config) {
        if ((config[0] >= '0') && (config[0] <= '9'))
            return atoi(config);
    }

    return default_value;
}

static char *drm_config_get_string_property(drm_device_t *dev, const char *name)
{
    char *env = getenv(name);
    char *config = NULL;

    /* Prefer using env variable */
    if (env) return env;

    if (dev->config)
        config = drm_config_find_value(dev, name);
    if (config) return config;

    return NULL;
}

static void drm_dump_config(drm_device_t *dev)
{
    drm_argv_t *argv = _lv_ll_get_head(&dev->arguments);
    while(argv) {
        LV_LOG_USER("key[%s]=value[%s]", argv->key, argv->value);
        argv = _lv_ll_get_next(&dev->arguments, argv);
    }
}

static void drm_parse_config(drm_device_t *dev)
{
    char *config = dev->config;
    char *key = NULL;
    char *value = NULL;
    drm_argv_t *argv;

    _lv_ll_init(&dev->arguments, sizeof(drm_argv_t));
    while (*config) {
        if (config_key_character_invalid(*config)) {
            config++;
            continue;
        }
        key = config;
        while (*config) {
            if (config_key_character_invalid(*config))
                break;
            config++;
        }
        if (*config != '=') {
            LV_LOG_WARN("Invalid key name [%.*s], ignore", config - key + 1, key);
            key = NULL;
            continue;
        }
        /* Break the string */
        *config = '\0';
        config++;
        while (*config) {
            if (!config_value_character_invalid(*config))
                break;
            config++;
        }
        if (!*config) {
            LV_LOG_WARN("Key [%s] has no value, ignore", key);
            break;
        }
        value = config;
        while (*config) {
            if (config_value_character_invalid(*config))
                break;
            config++;
        }
        if (!*config || (config != value)) {
            argv = _lv_ll_ins_tail(&dev->arguments);
            if (argv) {
                argv->key = key;
                argv->value = value;
            }
            key = NULL;
            value = NULL;
            if (*config) {
                /* String is not end, break the string */
                *config = '\0';
                config++;
            }
        }
    }
}

static void drm_config_file_init(drm_device_t *dev)
{
    FILE *fd;
    const char *path;
    int size;

    path = "/tmp/lv_drm_setup.cfg";
    if ((access(path, F_OK) != 0)) {
        path = "/userdata/lv_drm_setup.cfg";
        if ((access(path, F_OK) != 0))
            return;
    }

    fd = fopen(path, "rb");
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    dev->config = lv_malloc_zeroed(size + 1);
    if (!dev->config) {
        fclose(fd);
        return;
    }

    if (fread(dev->config, 1, size, fd) <= 0) {
        lv_free(dev->config);
        dev->config = NULL;
    }
    fclose(fd);

    if (dev->config) drm_parse_config(dev);
    if (dev->config) drm_dump_config(dev);
}

static void drm_config_file_deinit(drm_device_t *dev)
{
    if (dev && dev->config) {
        drm_argv_t *argv = _lv_ll_get_head(&dev->arguments);
        while(argv) {
            _lv_ll_remove(&dev->arguments, argv);
            lv_free(argv);
            argv = _lv_ll_get_head(&dev->arguments);
        }
        lv_free(dev->config);
        dev->config = NULL;
    }
}

static int drm_get_preferred_connector(void)
{
    const char *path;
    char buf[256] = "\0";
    int fd;

#define DRM_CONNECTOR_CFG_PATH_ENV	"DRM_CONNECTOR_CFG_PATH"
#define DRM_CONNECTOR_CFG_PATH_DEFAULT	"/tmp/drm_connector.cfg"
    path = getenv(DRM_CONNECTOR_CFG_PATH_ENV);
    if (!path)
        path = DRM_CONNECTOR_CFG_PATH_DEFAULT;

    if ((access(path, F_OK) == 0)) {
        fd = open(path, O_RDONLY);
        if (read(fd, buf, sizeof(buf)) <= 0)
            LV_LOG_TRACE("read failed, use default connector");
        close(fd);
    }

    if (!buf[0])
        return -1;

    return atoi(buf);
}

static int drm_get_preferred_mode(int *width, int *height)
{
    const char *path;
    char buf[256] = "\0";
    int fd, w, h;

#define DRM_MODE_CFG_PATH_ENV	"DRM_CONNECTOR_CFG_PATH"
#define DRM_MODE_CFG_PATH_DEFAULT	"/tmp/drm_mode.cfg"
    path = getenv(DRM_MODE_CFG_PATH_ENV);
    if (!path)
        path = DRM_MODE_CFG_PATH_DEFAULT;

    if ((access(path, F_OK) == 0)) {
        fd = open(path, O_RDONLY);
        if (read(fd, buf, sizeof(buf)) <= 0)
            LV_LOG_TRACE("read failed, use default mode");
        close(fd);
    }

    if (!buf[0])
        return -1;

    if (2 != sscanf(buf, "%dx%d", &w, &h))
        return -1;

    *width = w;
    *height = h;

    return 0;
}

static drmModeConnectorPtr
drm_get_connector(drm_device_t *dev, int connector_id)
{
    drmModeConnectorPtr conn;

    conn = drmModeGetConnector(dev->fd, connector_id);
    if (!conn)
        return NULL;

    LV_LOG_TRACE("Connector id: %d, %sconnected, modes: %d", connector_id,
              (conn->connection == DRM_MODE_CONNECTED) ? "" : "dis",
              conn->count_modes);
    if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes)
        return conn;

    drmModeFreeConnector(conn);
    return NULL;
}

static bool is_preferred_connector(const char *target, drmModeConnectorPtr conn)
{
    char full_name[32];
    int len1, len2;

    if (!target)
        return false;

    snprintf(full_name, sizeof(full_name), "%s-%u",
             drmModeGetConnectorTypeName(conn->connector_type), conn->connector_type_id);

    len1 = strlen(target);
    len2 = strlen(full_name);

    if (len1 > len2)
        return false;

    return (strncmp(target, full_name, len1) == 0) ? true : false;
}

static drmModeConnectorPtr
drm_find_best_connector(drm_device_t *dev)
{
    drmModeResPtr res = dev->res;
    drmModeConnectorPtr conn;
    int i, preferred_connector_id;
    char *preferred_connector_type = NULL;
    int fallback_id = -1;

    preferred_connector_id = drm_config_get_int_property(dev, "LV_DRM_CONNECTOR_ID",
                                                         drm_get_preferred_connector());
    LV_LOG_INFO("Preferred connector id: %d", preferred_connector_id);
    conn = drm_get_connector(dev, preferred_connector_id);
    if (conn)
        return conn;

    preferred_connector_type = drm_config_get_string_property(dev, "LV_DRM_CONNECTOR_NAME");
    LV_LOG_INFO("Preferred connector type: %s", preferred_connector_type);

    for (i = 0; i < res->count_connectors; i++) {
        conn = drm_get_connector(dev, res->connectors[i]);
        if (!conn)
            continue;
        if (preferred_connector_type)
        {
            if (is_preferred_connector(preferred_connector_type, conn))
                return conn;
            drmModeFreeConnector(conn);
            if (fallback_id == -1)
                fallback_id = i;
            continue;
        }
        return conn;
    }
    if (fallback_id != -1)
        return drm_get_connector(dev, res->connectors[fallback_id]);

    return NULL;
}

static drmModeCrtcPtr
drm_find_best_crtc(drm_device_t *dev, drmModeConnectorPtr conn)
{
    drmModeResPtr res = dev->res;
    drmModeEncoderPtr encoder;
    drmModeCrtcPtr crtc;
    int i, preferred_crtc_id = 0;
    int crtcs_for_connector = 0;

    encoder = drmModeGetEncoder(dev->fd, conn->encoder_id);
    if (encoder) {
        preferred_crtc_id = encoder->crtc_id;
        drmModeFreeEncoder(encoder);
        LV_LOG_TRACE("Preferred crtc: %d", preferred_crtc_id);
    }

    crtc = drmModeGetCrtc(dev->fd, preferred_crtc_id);
    if (crtc)
        return crtc;

    for (i = 0; i < res->count_encoders; i++) {
        encoder = drmModeGetEncoder(dev->fd, res->encoders[i]);
        if (encoder)
            crtcs_for_connector |= encoder->possible_crtcs;
        drmModeFreeEncoder(encoder);
    }
    LV_LOG_TRACE("Possible crtcs: %x", crtcs_for_connector);
    if (!crtcs_for_connector)
        return NULL;

    return drmModeGetCrtc(dev->fd, res->crtcs[ffs(crtcs_for_connector) - 1]);
}

static int drm_plane_get_int_property(drm_device_t *dev, int plane_id, char *prop_name)
{
    drmModeObjectPropertiesPtr props;
    drmModePropertyPtr prop;
    unsigned int i;
    int value = -1;

    props = drmModeObjectGetProperties(dev->fd, plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props)
        return -1;

    for (i = 0; i < props->count_props; i++) {
        prop = drmModeGetProperty(dev->fd, props->props[i]);
        if (!prop)
            continue;
        if (drmModeGetPropertyType(prop) == DRM_MODE_PROP_BLOB)
            continue;
        if (!strcmp(prop->name, prop_name)) {
            value = (int)props->prop_values[i];
            drmModeFreeProperty(prop);
            break;
        }
        drmModeFreeProperty(prop);
    }
    drmModeFreeObjectProperties(props);

    return value;
}

static int drm_plane_set_int_property(drm_device_t *dev, int plane_id, char *prop_name, int value)
{
    drmModeObjectPropertiesPtr props;
    drmModePropertyPtr prop = NULL;
    unsigned int prop_id;
    unsigned int i;
    bool value_invalid = true;

    props = drmModeObjectGetProperties(dev->fd, plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props)
        return -1;

    for (i = 0; i < props->count_props; i++) {
        prop = drmModeGetProperty(dev->fd, props->props[i]);
        if (!prop)
            continue;
	    if (prop->flags & DRM_MODE_PROP_IMMUTABLE)
            continue;
        if (drmModeGetPropertyType(prop) == DRM_MODE_PROP_BLOB)
            continue;
        if (!strcmp(prop->name, prop_name)) {
            prop_id = i;
            break;
        }
        drmModeFreeProperty(prop);
        prop = NULL;
    }

    if (prop) {
	    if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM)) {
            for (i = 0; i < prop->count_enums; i++) {
                if (prop->enums[i].value == value) {
                    value_invalid = false;
                    break;
                }
            }
        } else {
            value_invalid = false;
        }
	    if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE) ||
	        drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE)) {
            if (value < prop->values[0])
                value = prop->values[0];
            if (value > prop->values[1])
                value = prop->values[1];
        }
        if (!value_invalid)
            drmModeObjectSetProperty(dev->fd, plane_id, DRM_MODE_OBJECT_PLANE,
                                     props->props[prop_id], value);
        drmModeFreeProperty(prop);
    }

    drmModeFreeObjectProperties(props);

    return value;
}

static drmModePlanePtr get_preferred_plane(drm_device_t *dev, int plane_id, int pipe)
{
    drmModePlanePtr plane;
    char *preferred_plane;
    int preferred_type = -1;
    int type;

    plane = drmModeGetPlane(dev->fd, plane_id);
    if (!plane)
        return NULL;

    LV_LOG_TRACE("Check plane: %d, possible_crtcs: %x", plane_id,
                 plane->possible_crtcs);

    /* cannot used in current crtc */
    if (!(plane->possible_crtcs & (1 << pipe)))
        goto end;

    /* already in used */
    if (plane->fb_id)
        goto end;

    preferred_plane = drm_config_get_string_property(dev, "LV_DRM_PLANE_TYPE");
    if (!preferred_plane)
        preferred_plane = drm_config_get_string_property(dev, "LV_DRIVERS_SET_PLANE");
    if (preferred_plane) {
        if (!strcmp("OVERLAY", preferred_plane))
            preferred_type = DRM_PLANE_TYPE_OVERLAY;
        else if (!strcmp("PRIMARY", preferred_plane))
            preferred_type = DRM_PLANE_TYPE_PRIMARY;
        else if (!strcmp("CURSOR", preferred_plane))
            preferred_type = DRM_PLANE_TYPE_CURSOR;
    }

    if (preferred_type == -1)
        return plane;

    type = drm_plane_get_int_property(dev, plane_id, "type");
    if (type == -1) {
        LV_LOG_ERROR("Get type property failed");
        goto end;
    }

    if (type == preferred_type)
        return plane;

end:
    drmModeFreePlane(plane);
    return NULL;
}

static drmModePlanePtr
drm_find_best_plane(drm_device_t *dev, drmModeCrtcPtr crtc)
{
    drmModeResPtr res = dev->res;
    drmModePlaneResPtr pres;
    drmModePlanePtr plane = NULL;
    int preferred_plane_id;
    unsigned int i;
    int pipe;

    preferred_plane_id = drm_config_get_int_property(dev, "LV_DRM_PLANE_ID", 0);
    if (preferred_plane_id) {
        plane = drmModeGetPlane(dev->fd, preferred_plane_id);
        if (plane)
            return plane;
    }

    for (pipe = 0; pipe < res->count_crtcs; pipe++) {
        if (crtc->crtc_id == res->crtcs[pipe])
            break;
    }
    if (pipe == res->count_crtcs)
        return NULL;

    pres = drmModeGetPlaneResources(dev->fd);
    if (!pres)
        return NULL;

    for (i = 0; i < pres->count_planes; i++) {
        plane = get_preferred_plane(dev, pres->planes[i], pipe);
        if (plane)
            break;
    }

    drmModeFreePlaneResources(pres);

    return plane;
}

static drmModeModeInfoPtr
drm_find_best_mode(drm_device_t *dev, drmModeConnectorPtr conn)
{
    drmModeModeInfoPtr mode;
    int i, preferred_width, preferred_height;
    int best_i = 0, max_width = 0, max_height = 0;

    if (dev == NULL)
        return 0;

    if (drm_get_preferred_mode(&preferred_width, &preferred_height) == -1) {
        preferred_width = drm_config_get_int_property(dev, "LV_DRM_MODE_PREFERD_WIDTH",
                                                      dev->preferred_width);
        preferred_height = drm_config_get_int_property(dev, "LV_DRM_MODE_PREFERD_HEIGHT",
                                                       dev->preferred_height);
    }
    LV_LOG_TRACE("Preferred mode: %dx%d", preferred_width, preferred_height);

    for (i = 0; i < conn->count_modes; i++) {
        LV_LOG_TRACE("Check mode: %dx%d",
                conn->modes[i].hdisplay, conn->modes[i].vdisplay);
        if (conn->modes[i].hdisplay == preferred_width &&
                conn->modes[i].vdisplay == preferred_height) {
            best_i = i;
            break;
        }
        if (conn->modes[i].hdisplay > max_width &&
                conn->modes[i].vdisplay > max_height) {
            best_i = i;
            max_width = conn->modes[i].hdisplay;
            max_height = conn->modes[i].vdisplay;
        }
    }
    mode = &conn->modes[best_i];

    return mode;
}

static void drm_free(drm_device_t * dev)
{
    int i;

    if (dev->res) {
        drmModeFreeResources(dev->res);
        dev->res = NULL;
    }

    dev->connector_id = 0;
    dev->crtc_id = 0;
    dev->plane_id = 0;
    dev->mode.physical_width = 0;
    dev->mode.physical_height = 0;
}

static int drm_setup(drm_device_t *dev)
{
    drmModeConnectorPtr conn = NULL;
    drmModeModeInfoPtr mode;
    drmModePlanePtr plane = NULL;
    drmModeCrtcPtr crtc = NULL;
    drm_bo_t *crtc_bo;
    int type;
    int zpos;
    int ret;
    int i, success = 0;

    dev->res = drmModeGetResources(dev->fd);
    if (!dev->res) {
        LV_LOG_ERROR("drm get resource failed");
        goto err;
    }

    conn = drm_find_best_connector(dev);
    if (!conn) {
        LV_LOG_ERROR("drm find connector failed");
        goto err;
    }
    LV_LOG_USER("Best connector id: %d", conn->connector_id);

    crtc = drm_find_best_crtc(dev, conn);
    if (!crtc) {
        LV_LOG_ERROR("drm find crtc failed");
        goto err;
    }
    LV_LOG_USER("Best crtc: %d", crtc->crtc_id);
    if (crtc->mode_valid) {
        dev->preferred_width = crtc->mode.hdisplay;
        dev->preferred_height = crtc->mode.vdisplay;
    }

    mode = drm_find_best_mode(dev, conn);
    if (!mode) {
        LV_LOG_ERROR("drm find mode failed");
        goto err;
    }
    LV_LOG_USER("Best mode: %dx%d", mode->hdisplay, mode->vdisplay);
    memcpy(&dev->mode_info, mode, sizeof(dev->mode_info));

    plane = drm_find_best_plane(dev, crtc);
    if (!plane) {
        LV_LOG_ERROR("drm find plane failed");
        goto err;
    }

    type = drm_plane_get_int_property(dev, plane->plane_id, "type");
    if (type == DRM_PLANE_TYPE_PRIMARY)
        dev->is_primary_plane = true;

    zpos = drm_config_get_int_property(dev, "LV_DRM_PLANE_ZPOS", -1);
    if (zpos > 0)
        drm_plane_set_int_property(dev, plane->plane_id, "zpos", zpos);
    drm_plane_set_int_property(dev, plane->plane_id, "pixel blend mode", 1);
    LV_LOG_USER("Best plane: %d", plane->plane_id);
    dev->connector_id = conn->connector_id;
    dev->crtc_id = crtc->crtc_id;
    dev->plane_id = plane->plane_id;
    dev->mode.physical_width = mode->hdisplay;
    dev->mode.physical_height = mode->vdisplay;
    dev->vsync = drm_config_get_int_property(dev, "LV_DRM_VSYNC", 1);

    crtc_bo = malloc_drm_bo(dev, dev->mode.physical_width, dev->mode.physical_height, DRM_FORMAT);
    if (crtc_bo)
    {
        uint32_t fps = drm_config_get_int_property(dev, "lv_disp_fps", 0);
	    uint32_t con_ids[1];
        if (fps && (fps != dev->mode_info.vrefresh))
        {
            LV_LOG_USER("Update FPS %d -> %d", dev->mode_info.vrefresh, fps);
            dev->mode_info.clock = dev->mode_info.clock * fps / dev->mode_info.vrefresh;
            dev->mode_info.vrefresh = fps;
        }
        memset(crtc_bo->ptr, 0x0, crtc_bo->size);
        con_ids[0] = dev->connector_id;
        ret = drmModeSetCrtc(dev->fd, dev->crtc_id, crtc_bo->fb_id, 0, 0,
                             con_ids, 1, &dev->mode_info);
        if (ret)
            LV_LOG_WARN("drmModeSetCrtc failed %d %d", dev->crtc_id, ret);
        free_drm_bo(dev, crtc_bo);
    }
    success = 1;
err:
    drmModeFreeConnector(conn);
    drmModeFreePlane(plane);
    drmModeFreeCrtc(crtc);
    if (!success) {
        drm_free(dev);
        return -1;
    }
    return 0;
}

static void drm_flip_handler(int fd, unsigned frame, unsigned sec,
                             unsigned usec, void *data)
{
    // data is &dev->waiting_for_flip
    LV_LOG_TRACE("Page flip received(%d)!, %d, %d, %d, %d", *(int*)data, fd, frame, sec, usec);
    *(int*)data = 0;
}

static drm_device_t * drm_init(void)
{
    drm_device_t * dev;
    int ret;

    dev = lv_malloc_zeroed(sizeof(drm_device_t));
    if (dev == NULL) {
        LV_LOG_ERROR("allocate device failed");
        return NULL;
    }

    dev->fd = drmOpen("rockchip", NULL);
    if (dev->fd < 0)
        dev->fd = open("/dev/dri/card0", O_RDWR);
    if (dev->fd < 0) {
        LV_LOG_ERROR("drm open failed");
        goto err_drm_open;
    }
    fcntl(dev->fd, F_SETFD, FD_CLOEXEC);

    drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    drm_config_file_init(dev);

    ret = drm_setup(dev);
    if (ret) {
        LV_LOG_ERROR("drm setup failed");
        goto err_drm_setup;
    }

    dev->drm_pollfd.fd = dev->fd;
    dev->drm_pollfd.events = POLLIN;

    dev->drm_evctx.version = DRM_EVENT_CONTEXT_VERSION;
    dev->drm_evctx.page_flip_handler = drm_flip_handler;

    return dev;

err_alloc_fb:
    drm_free(dev);
err_drm_setup:
    drmClose(dev->fd);
err_drm_open:
    lv_free(dev);

    return NULL;
}

static void drm_deinit(drm_device_t * dev)
{
    drm_free(dev);
    drm_config_file_deinit(dev);

    if (dev->fd > 0)
        drmClose(dev->fd);
}

static void drm_wait_flip(drm_device_t* dev, int timeout)
{
    int ret;

    while (dev->waiting_for_flip) {
        dev->drm_pollfd.revents = 0;
        ret = poll(&dev->drm_pollfd, 1, timeout);
        if (ret <= 0)
            return;

        drmHandleEvent(dev->fd, &dev->drm_evctx);
    }
}

static void drm_set_plane(drm_device_t* dev, drm_bo_t *bo)
{
    int crtc_x, crtc_y, crtc_w, crtc_h;
    int ret;
    int fb = bo->fb_id, sw = dev->mode.width, sh = dev->mode.height;
    int flip_fb;
    drmVBlank vbl;

    if (dev == NULL)
        return;

    if (dev->disp_crop)
    {
        crtc_w = sw;
        crtc_h = sh;
        crtc_x = (dev->mode.physical_width - sw) / 2;
        crtc_y = (dev->mode.physical_height - sh) / 2;
    }
    else
    {
        /* Fullscreen */
        crtc_w = dev->mode.physical_width;
        crtc_h = dev->mode.physical_height;
        crtc_x = 0;
        crtc_y = 0;
    }

    LV_LOG_TRACE("Display bo %d(%dx%d) at (%d,%d) %dx%d", fb, sw, sh,
                 crtc_x, crtc_y, crtc_w, crtc_h);
    ret = drmModeSetPlane(dev->fd, dev->plane_id, dev->crtc_id, fb, 0,
                          crtc_x, crtc_y, crtc_w, crtc_h,
                          0, 0, sw << 16, sh << 16);
    if (ret) {
        LV_LOG_ERROR("drm set plane failed");
        return;
    }
    if (dev->vsync) {
        if (dev->vsync == 1) {
            if (dev->is_primary_plane) {
                flip_fb = fb;
            } else {
                drmModeCrtc *crtc = drmModeGetCrtc(dev->fd, dev->crtc_id);
                if (!crtc)
                    return;
                flip_fb = crtc->buffer_id;
                drmModeFreeCrtc (crtc);
            }
            // Queue page flip
            dev->waiting_for_flip = 1;
            ret = drmModePageFlip(dev->fd, dev->crtc_id, flip_fb,
                                  DRM_MODE_PAGE_FLIP_EVENT, &dev->waiting_for_flip);
            if (ret) {
                LV_LOG_ERROR("drm page flip failed %d", ret);
                return;
            }
        } else {
            memset(&vbl, 0, sizeof(vbl));
            vbl.request.type = (drmVBlankSeqType)(DRM_VBLANK_RELATIVE);
            vbl.request.sequence = 1;
            vbl.request.signal = &dev->waiting_for_flip;
            ret = drmWaitVBlank(dev->fd, &vbl);
            if (ret) {
                LV_LOG_ERROR("drm wait vblank failed %d", ret);
                return;
            }
        }
        // Wait for last page flip
        drm_wait_flip(dev, -1);
    }
}

#if LV_DRM_USE_RGA
static void drm_overlay(drm_device_t * dev, drm_bo_t *bo)
{
    if (dev->overlay.fd)
    {
        rga_buffer_t src_img, dst_img, pat_img;
        im_rect src_rect, dst_rect, pat_rect;
        int src_fd = dev->overlay.fd;
        int dst_fd = bo->buf_fd;
        int usage = IM_SYNC;
        int ret;

        switch (dev->disp_rot)
        {
        case 90:
            dst_rect.x = dev->overlay.dst_y;
            dst_rect.y = dev->overlay.dst_x;
            dst_rect.width = dev->overlay.h;
            dst_rect.height = dev->overlay.w;
            usage |= IM_HAL_TRANSFORM_ROT_90;
            break;
        case 180:
            dst_rect.x = dev->overlay.dst_x;
            dst_rect.y = dev->overlay.dst_y;
            dst_rect.width = dev->overlay.w;
            dst_rect.height = dev->overlay.h;
            usage |= IM_HAL_TRANSFORM_ROT_180;
            break;
        case 270:
            dst_rect.x = dev->overlay.dst_y;
            dst_rect.y = dev->overlay.dst_x;
            dst_rect.width = dev->overlay.h;
            dst_rect.height = dev->overlay.w;
            usage |= IM_HAL_TRANSFORM_ROT_270;
            break;
        default:
            break;
        }

        src_img = wrapbuffer_fd(src_fd, dev->overlay.w,
                                dev->overlay.h, RGA_FORMAT,
                                dev->overlay.vw, dev->overlay.vh);
        dst_img = wrapbuffer_fd(dst_fd, bo->w,
                                bo->h, RGA_FORMAT,
                                bo->pitch / (LV_COLOR_DEPTH >> 3),
                                bo->h);
        src_rect.x = dev->overlay.ofs_x;
        src_rect.y = dev->overlay.ofs_y;
        src_rect.width = dev->overlay.w;
        src_rect.height = dev->overlay.h;
//        printf("%s %d %d %d %d %d %d %d %d\n", __func__,
//               src_rect.x, src_rect.y, src_rect.width, src_rect.height,
//               dst_rect.x, dst_rect.y, dst_rect.width, dst_rect.height);
        memset(&pat_img, 0, sizeof(pat_img));
        memset(&pat_rect, 0, sizeof(pat_rect));
        usage |= (IM_ALPHA_BLEND_SRC_OVER | IM_ALPHA_BLEND_PRE_MUL);
        ret = imcheck_composite(src_img, dst_img, pat_img,
                                src_rect, dst_rect, pat_rect);
        if (ret != IM_STATUS_NOERROR)
        {
            LV_LOG_ERROR("%d, check error! %s\n", __LINE__,
                         imStrError((IM_STATUS)ret));
        }
        else
        {
            ret = improcess(src_img, dst_img, pat_img,
                            src_rect, dst_rect, pat_rect, usage);
            if (ret != IM_STATUS_SUCCESS)
                LV_LOG_ERROR("%d, running failed, %s\n", __LINE__,
                             imStrError((IM_STATUS)ret));
        }
    }
}
#endif

static void *drm_thread(void *arg)
{
    drm_device_t * dev = (drm_device_t *)arg;
    drm_bo_t *bo = NULL;
    drm_bo_t *vop_buf[2];
#if LV_DRM_USE_RGA
    drm_bo_t *gbo = dev->gbo;
    rga_buffer_t vop_img[2];
    rga_buffer_t src_img, dst_img, pat_img;
    im_rect src_rect, dst_rect, pat_rect;
    int usage = IM_SYNC;
    int src_fd;
    int dst_fd;
    int ret;
#endif

    vop_buf[0] = dev->vop_buf[0];
    vop_buf[1] = dev->vop_buf[1];

#if LV_DRM_USE_RGA
    switch (dev->disp_rot)
    {
    case 90: usage |= IM_HAL_TRANSFORM_ROT_90; break;
    case 180: usage |= IM_HAL_TRANSFORM_ROT_180; break;
    case 270: usage |= IM_HAL_TRANSFORM_ROT_270; break;
    default: break;
    }

    src_img = wrapbuffer_fd(gbo->buf_fd, gbo->w,
                            gbo->h, RGA_FORMAT,
                            gbo->pitch / (LV_COLOR_DEPTH >> 3),
                            gbo->h);
    vop_img[0] = wrapbuffer_fd(vop_buf[0]->buf_fd, vop_buf[0]->w,
                               vop_buf[0]->h, RGA_FORMAT,
                               vop_buf[0]->pitch / (LV_COLOR_DEPTH >> 3),
                               vop_buf[0]->h);
    vop_img[1] = wrapbuffer_fd(vop_buf[1]->buf_fd, vop_buf[1]->w,
                               vop_buf[1]->h, RGA_FORMAT,
                               vop_buf[1]->pitch / (LV_COLOR_DEPTH >> 3),
                               vop_buf[1]->h);
    memset(&pat_img, 0, sizeof(pat_img));
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&pat_rect, 0, sizeof(pat_rect));
#endif

    while (!dev->quit) {
        pthread_mutex_lock(&dev->mutex);
        if (dev->draw_update) {
            bo = (bo == vop_buf[0]) ? vop_buf[1] : vop_buf[0];

#if LV_DRM_USE_RGA
            dst_img = (bo == vop_buf[0]) ? vop_img[0] : vop_img[1];
            ret = imcheck_composite(src_img, dst_img, pat_img,
                                    src_rect, dst_rect, pat_rect);
            if (ret != IM_STATUS_NOERROR)
            {
                LV_LOG_ERROR("%d, check error! %s\n", __LINE__,
                             imStrError((IM_STATUS)ret));
            }
            else
            {
                ret = improcess(src_img, dst_img, pat_img,
                                src_rect, dst_rect, pat_rect, usage);
                if (ret != IM_STATUS_SUCCESS)
                    LV_LOG_ERROR("%d, running failed, %s\n", __LINE__,
                                 imStrError((IM_STATUS)ret));
            }
            drm_overlay(dev, bo);
#else
            for (int i = 0; i < dev->mode.height; i++)
            {
                memcpy(bo->ptr + i * bo->pitch,
                       dev->flush_buf + i * dev->flush_buf_stride * (LV_COLOR_DEPTH >> 3),
                       bo->pitch);
            }
#endif
            drm_set_plane(dev, bo);
            dev->draw_update = 0;
        }
        pthread_mutex_unlock(&dev->mutex);
        usleep(100);
    }
    return NULL;
}

#if SW_ROTATION
#if (LV_COLOR_DEPTH == 16)
typedef uint16_t data_type;
#elif (LV_COLOR_DEPTH == 32)
typedef uint32_t data_type;
#endif

static void rotate_90(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    drm_device_t * dev = lv_display_get_driver_data(disp);
    data_type *src = (data_type *)color_p;
    data_type *dst;
    int32_t x, tx;
    int32_t y, ty;

    for(y = area->y1; y <= area->y2; y++) {
        tx = (dev->mode.width - 1) - y;
        ty = area->x1;
        dst = (data_type *)(dev->flush_buf + (ty * dev->flush_buf_stride + tx) * (LV_COLOR_DEPTH >> 3));
        for(x = area->x1; x <= area->x2; x++) {
            *dst = *src;
            src++;
            dst += dev->flush_buf_stride;
        }
    }
}

static void rotate_180(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    drm_device_t * dev = lv_display_get_driver_data(disp);
    data_type *src = (data_type *)color_p;
    data_type *dst;
    int32_t x, tx;
    int32_t y, ty;

    for(y = area->y1; y <= area->y2; y++) {
        tx = (dev->mode.width - 1) - area->x1;
        ty = (dev->mode.height - 1) - y;
        dst = (data_type *)(dev->flush_buf + (ty * dev->flush_buf_stride + tx) * (LV_COLOR_DEPTH >> 3));
        for(x = area->x1; x <= area->x2; x++) {
            *dst = *src;
            src++;
            dst--;
        }
    }
}

static void rotate_270(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    drm_device_t * dev = lv_display_get_driver_data(disp);
    data_type *src = (data_type *)color_p;
    data_type *dst;
    int32_t x, tx;
    int32_t y, ty;

    for(y = area->y1; y <= area->y2; y++) {
        tx = y;
        ty = (dev->mode.height - 1) - area->x1;
        dst = (data_type *)(dev->flush_buf + (ty * dev->flush_buf_stride + tx) * (LV_COLOR_DEPTH >> 3));
        for(x = area->x1; x <= area->x2; x++) {
            *dst = *src;
            src++;
            dst -= dev->flush_buf_stride;
        }
    }
}
#endif

static void drm_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    drm_device_t * dev = lv_display_get_driver_data(disp);
    int32_t x;
    int32_t y;
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    pthread_mutex_lock(&dev->mutex);

#if SW_ROTATION
    if (!dev->disp_rot)
    {
#endif
        for(y = area->y1; y <= area->y2; y++) {
            lv_color_t *ptr = (lv_color_t*)(dev->flush_buf + (y * dev->flush_buf_stride + area->x1) * (LV_COLOR_DEPTH >> 3));
            memcpy(ptr, color_p, w * (LV_COLOR_DEPTH >> 3));
            color_p += w * (LV_COLOR_DEPTH >> 3);
        }
#if SW_ROTATION
    }
    else
    {
        if (dev->disp_rot == 90)
            rotate_90(disp, area, color_p);
        else if (dev->disp_rot == 180)
            rotate_180(disp, area, color_p);
        else if (dev->disp_rot == 270)
            rotate_270(disp, area, color_p);
    }
#endif
    if(lv_display_flush_is_last(disp))
        dev->draw_update = 1;
    pthread_mutex_unlock(&dev->mutex);

    lv_display_flush_ready(disp);
}

static void drm_buffer_setup(drm_device_t * dev)
{
    int buf_w, buf_h;

    LV_LOG_USER("bit depth %d", LV_COLOR_DEPTH);

    /* If use software rotation, the size of flush buffer equal to vop buffer,
     * If use RGA rotaion, the size of flush buffer equal to draw buffer.
     * lvgl->draw buffer->flush buffer->vop buffer
     */
#if SW_ROTATION
    buf_w = dev->mode.width;
    buf_h = dev->mode.height;
#else
    if ((dev->disp_rot == 0) || (dev->disp_rot == 180))
    {
        buf_w = dev->mode.width;
        buf_h = dev->mode.height;
    }
    else
    {
        buf_w = dev->mode.height;
        buf_h = dev->mode.width;
    }
#endif
#if LV_DRM_USE_RGA
    dev->gbo = malloc_drm_bo(dev, buf_w, buf_h, DRM_FORMAT);
    dev->flush_buf = dev->gbo->ptr;
    dev->flush_buf_stride = dev->gbo->pitch / (LV_COLOR_DEPTH >> 3);
    c_RkRgaInit();
#else
    dev->flush_buf = malloc(buf_w * buf_h * (LV_COLOR_DEPTH >> 3));
    dev->flush_buf_stride = buf_w;
#endif
    dev->vop_buf[0] = malloc_drm_bo(dev, dev->mode.width, dev->mode.height, DRM_FORMAT);
    dev->vop_buf[1] = malloc_drm_bo(dev, dev->mode.width, dev->mode.height, DRM_FORMAT);
    LV_LOG_USER("DRM subsystem and buffer mapped successfully");
}

static void drm_buffer_destroy(drm_device_t * dev)
{
#if LV_DRM_USE_RGA
    free_drm_bo(dev, dev->gbo);
#else
    free(dev->flush_buf);
    free(dev->disp_buf);
#endif
    free_drm_bo(dev, dev->vop_buf[0]);
    free_drm_bo(dev, dev->vop_buf[1]);
}

lv_display_t * lv_drm_disp_create(int hor_res, int ver_res, int rot)
{
    lv_display_t * disp;

    drm_device_t * dev = drm_init();
    if (!dev)
        return NULL;
#if LV_DRM_USE_RGA || SW_ROTATION
    dev->disp_rot = drm_config_get_int_property(dev, "lv_disp_rot", rot);;
#endif
    dev->mode.width = drm_config_get_int_property(dev, "lv_disp_width", hor_res);
    dev->mode.height = drm_config_get_int_property(dev, "lv_disp_height", ver_res);
    dev->disp_crop = drm_config_get_int_property(dev, "lv_disp_crop", 0);
    if (!dev->mode.width)
        dev->mode.width = dev->mode.physical_width;
    if (!dev->mode.height)
        dev->mode.height = dev->mode.physical_height;

    drm_buffer_setup(dev);

    if ((dev->disp_rot == 0) || (dev->disp_rot == 180))
        disp = lv_display_create(dev->mode.width, dev->mode.height);
    else
        disp = lv_display_create(dev->mode.height, dev->mode.width);

    if (disp == NULL) {
        LV_LOG_ERROR("lv_display_create failed");
        return NULL;
    }

    int size = dev->mode.width * dev->mode.height * (LV_COLOR_DEPTH >> 3);
    dev->disp_buf = lv_malloc(size);
    lv_display_set_buffers(disp, dev->disp_buf, NULL, size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_driver_data(disp, dev);
    lv_display_set_flush_cb(disp, drm_flush);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA);

    pthread_mutex_init(&dev->mutex, NULL);
    pthread_create(&dev->pid, NULL, drm_thread, dev);

    return disp;
}

int lv_drm_disp_delete(lv_display_t * disp)
{
    drm_device_t* dev = lv_display_get_driver_data(disp);

    if (!dev)
        goto end;

    dev->quit = 0;
    pthread_join(dev->pid, NULL);

    drm_buffer_destroy(dev);

    drm_deinit(dev);

    lv_free(dev->disp_buf);
    lv_free(dev);
end:
    lv_display_delete(disp);

    return 0;
}

overlay_dma_buffer_t *lv_drm_disp_create_overlay(lv_display_t * disp,
                                                 int w, int h)
{
    drm_device_t* dev = lv_display_get_driver_data(disp);
    overlay_dma_buffer_t * overlay;
    drm_bo_t *bo;

    overlay = lv_malloc_zeroed(sizeof(overlay_dma_buffer_t));
    if (!overlay)
    {
        LV_LOG_ERROR("Create overlay failed");
        return NULL;
    }

    bo = malloc_drm_bo(dev, w, h, DRM_FORMAT);
    if (!bo)
    {
        LV_LOG_ERROR("Create bo failed");
        lv_free(overlay);
        return NULL;
    }

    overlay->user_data = bo;
    overlay->fd = bo->buf_fd;
    overlay->data = bo->ptr;
    overlay->stride = bo->pitch / (LV_COLOR_DEPTH >> 3);

    return overlay;
}

void lv_drm_disp_destroy_overlay(lv_display_t * disp,
                                 overlay_dma_buffer_t * overlay)
{
    drm_device_t* dev = lv_display_get_driver_data(disp);
    drm_bo_t *bo;

    if (!dev)
        return;

    if (!overlay)
        return;

    bo = overlay->user_data;
    if (!bo)
        return;

    free_drm_bo(dev, bo);
    lv_free(overlay);
}

void lv_drm_disp_set_overlay(lv_display_t * disp,
                             overlay_dma_buffer_t * overlay)
{
#if LV_DRM_USE_RGA
    drm_device_t* dev = lv_display_get_driver_data(disp);

    if (!dev)
        return;

    if (!overlay)
    {
        memset(&dev->overlay, 0, sizeof(overlay_dma_buffer_t));
        return;
    }
    memcpy(&dev->overlay, overlay, sizeof(overlay_dma_buffer_t));
#else
    LV_LOG_WARN("No support overlay without RGA");
#endif
}

#endif
