#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>

#include <errno.h>

#include "drm.h"
#include "log.h"

static int fd = -1;

struct type_name {
    unsigned int type;
    const char *name;
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static const struct type_name connector_type_names[] = {
    { DRM_MODE_CONNECTOR_Unknown,     "unknown"   },
    { DRM_MODE_CONNECTOR_VGA,         "VGA"       },
    { DRM_MODE_CONNECTOR_DVII,        "DVI-I"     },
    { DRM_MODE_CONNECTOR_DVID,        "DVI-D"     },
    { DRM_MODE_CONNECTOR_DVIA,        "DVI-A"     },
    { DRM_MODE_CONNECTOR_Composite,   "composite" },
    { DRM_MODE_CONNECTOR_SVIDEO,      "s-video"   },
    { DRM_MODE_CONNECTOR_LVDS,        "LVDS"      },
    { DRM_MODE_CONNECTOR_Component,   "component" },
    { DRM_MODE_CONNECTOR_9PinDIN,     "9-pin DIN" },
    { DRM_MODE_CONNECTOR_DisplayPort, "DP"        },
    { DRM_MODE_CONNECTOR_HDMIA,       "HDMI-A"    },
    { DRM_MODE_CONNECTOR_HDMIB,       "HDMI-B"    },
    { DRM_MODE_CONNECTOR_TV,          "TV"        },
    { DRM_MODE_CONNECTOR_eDP,         "eDP"       },
    { DRM_MODE_CONNECTOR_VIRTUAL,     "Virtual"   },
    { DRM_MODE_CONNECTOR_DSI,         "DSI"       },
    { DRM_MODE_CONNECTOR_DPI,         "DPI"       },
};

static const char *connector_type_name(unsigned int type)
{
    if (type < ARRAY_SIZE(connector_type_names))
        return connector_type_names[type].name;

    return "INVALID";
}

struct color_info {
    uint8_t size;
    uint8_t offset;
};

struct pixel_info {
    uint32_t format_pixel;
    unsigned int depth;
    unsigned int bpp;
    struct color_info rgba[4];
};

static const struct pixel_info format_pixels[] = {
    { DRM_FORMAT_XRGB8888, 24, 32, {{ 8, 16 }, { 8, 8 }, { 8, 0 }, { 0, 0 }} },
};

static char *pixel_format_fourcc(uint32_t pixel_format)
{
    static char fourcc[5];

    fourcc[0] = (pixel_format >> 0)  & 0xFF;
    fourcc[1] = (pixel_format >> 8)  & 0xFF;
    fourcc[2] = (pixel_format >> 16) & 0xFF;
    fourcc[3] = (pixel_format >> 24) & 0xFF;
    fourcc[4] = 0;

    return fourcc;
}

static int get_format_pixel_info(uint32_t format_pixel, unsigned int *depth,
                                 unsigned int *bpp, struct color_info rgba[4])
{
    unsigned int i;

    for (i = 0; i < sizeof(format_pixels) / sizeof(format_pixels[0]); i++) {
        if (format_pixels[i].format_pixel == format_pixel) {
            *depth = format_pixels[i].depth;
            *bpp = format_pixels[i].bpp;
            rgba[0] = format_pixels[i].rgba[0];
            rgba[1] = format_pixels[i].rgba[1];
            rgba[2] = format_pixels[i].rgba[2];
            rgba[3] = format_pixels[i].rgba[3];
            return 0;
        }
    }

    errno = EINVAL;
    return -1;
}

static int init_drm(const char *device)
{
    drmModeFB2        *fb;
    drmModeRes        *ressource;
    drmModeCrtc       *crtc;
    drmModeConnector  *connector = NULL;
    drmModeEncoder    *encoder = NULL;
    drmModeModeInfo   *drmResolution = NULL;
    struct color_info rgba[4];
    unsigned int      bpp;
    unsigned int      depth;

    log_info("Initializing DRM device %s ...\n", device);

    /* Open the DRM device */
    fd = open(device, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        log_error("Unable to open DRM device %s: %m\n", device);
        return 0;
    }

    /* Retrieve device resources */
    ressource = drmModeGetResources(fd);
    if (!ressource) {
        log_fatal("Unable to retrieve DRM resources: %m.\n");
        return 0;
    }

    /* No connector */
    if (ressource->count_connectors < 1) {
        log_fatal("No connector found for that device.\n");
        return 0;
    }

    /* Use the first connector */
    connector = drmModeGetConnectorCurrent(fd, ressource->connectors[0]);
    if (!connector) {
        log_fatal("Unable to get DRM device connector[0].\n");
        return 0;
    }
    log_info("DRM Device Name: %s-%u\n",
             connector_type_name(connector->connector_type),
             connector->connector_type_id);
    log_info("Encoder        : %d\n", connector->encoder_id);

    /* No mode */
    if (connector->count_modes <= 0) {
        log_fatal("No modes found for this connector.\n");
        return 0;
    }

    /*
     * Get resolution (we do not check preferred because there is only one
     * mode)
     */
    drmResolution = &connector->modes[0];
    log_info("Resolution : %ux%u@%u\n", drmResolution->hdisplay,
             drmResolution->vdisplay, drmResolution->vrefresh);
    log_info("(ht: %u hs: %u he: %u hskew: %u, vt: %u vs: %u ve: %u vscan: %u, flags: 0x%X %s)\n",
             drmResolution->htotal, drmResolution->hsync_start,
             drmResolution->hsync_end, drmResolution->hskew,
             drmResolution->vtotal, drmResolution->vsync_start,
             drmResolution->vsync_end, drmResolution->vscan,
             drmResolution->flags,
             drmResolution->type & DRM_MODE_TYPE_PREFERRED ? "<P>":"");

    encoder = drmModeGetEncoder(fd, connector->encoder_id);
    if (!encoder) {
        log_fatal("Unable to retrieve DRM encoder: %m.\n");
        return 0;
    }

    crtc = drmModeGetCrtc(fd, encoder->crtc_id);
    if (!crtc) {
        log_fatal("Unable to retrieve DRM CRTC: %m\n");
        return 0;
    }

    log_info("Connector %d is connected to encoder %d CRTC %d.\n",
             connector->connector_id, connector->encoder_id,
             crtc->crtc_id);

    /* Check framebuffer id */
    fb = drmModeGetFB2(fd, crtc->buffer_id);
    if (fb == NULL) {
        log_fatal("Unable to retrieve DRM frame buffer: %m\n");
        return 0;
    }

    log_info("Got framebuffer at CRTC: %d.\n", crtc->crtc_id);
    log_info("FB is format %s width %u height %u.\n",
             pixel_format_fourcc(fb->pixel_format), fb->width,
             fb->height);

    /* Frame buffer has multiple planes */
    if (fb->handles[1]) {
        int i;
        int planes = 1;

        for (i = 1; i < 4; i++)
            if (fb->handles[i])
                planes++;

        log_info("FB has %d plane(s):\n", planes);
        for (i = 0; i < 4; i++)
            if (fb->handles[i])
                log_info("Plane %d has handle %u pitch %u offset %u\n", i,
                         fb->handles[i], fb->pitches[i],
                         fb->offsets[i]);
    }

    /* Frame buffer has modifier */
    if (fb->modifier == I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC)
        log_warn("FB has modifier Intel Color Control Surface with Clear Color (CCS) for DG2 render compression.\n");
    else if (fb->modifier)
        log_warn("FB mas modifier 0x%016llx.\n",
                 (unsigned long long)fb->modifier);

    /*
     * Now this is how we dump the framebuffer
     * structure to retrieve FB later
     */
    struct drm_mode_map_dumb dumb_map;

    memset(&dumb_map, 0, sizeof(dumb_map));
    dumb_map.handle = fb->handles[0];
    dumb_map.offset = 0;

    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &dumb_map) != 0) {
        log_fatal("Unable to ioctl (DRM_IOCTL_MODE_MAP_DUMB): %m\n");
        return 0;
    }

    /* Recompute with drm infos..should be the same as fb0 */
    if (get_format_pixel_info(fb->pixel_format, &depth, &bpp, rgba) != 0) {
        log_fatal("Unsupported format %x: %m\n", fb->pixel_format);
        return 0;
    }
    FrameBuffer_Size         = fb->pitches[0] * fb->height;
    FrameBuffer_Xwidth       = fb->width;
    FrameBuffer_Yheight      = fb->height;
    FrameBuffer_BitsPerPixel = bpp;
    FrameBuffer_Depth        = depth;
    FrameBuffer_RedMax       = (1 << rgba[0].size) - 1;
    FrameBuffer_GreenMax     = (1 << rgba[1].size) - 1;
    FrameBuffer_BlueMax      = (1 << rgba[2].size) - 1;
    FrameBuffer_RedShift     = rgba[0].offset;
    FrameBuffer_GreenShift   = rgba[1].offset;
    FrameBuffer_BlueShift    = rgba[2].offset;
    log_info(" fb xres=%d, yres=%d, bpp=%d\n",
             (int)FrameBuffer_Xwidth, (int)FrameBuffer_Yheight,
             (int)FrameBuffer_BitsPerPixel);

    log_info("  offset:length red=%d:%d green=%d:%d blue=%d:%d \n",
             (int)FrameBuffer_RedShift, (int)FrameBuffer_RedMax,
             (int)FrameBuffer_GreenShift, (int)FrameBuffer_GreenMax,
             (int)FrameBuffer_BlueShift, (int)FrameBuffer_BlueMax);

    log_info("  frame buffer size : %d bytes\n", FrameBuffer_Size);

    if (bpp != 32) {
        log_fatal("Only 32 bits framebuffer is supported. Current Bits Per Pixel is %d.\n",
                  bpp);
        exit(EXIT_FAILURE);
    }

    DRM_FrameBuffer = mmap(0, FrameBuffer_Size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd, dumb_map.offset);
    if (DRM_FrameBuffer == MAP_FAILED) {
        log_fatal("Unable to mmap DRM frame buffer: %m\n");
        exit(EXIT_FAILURE);
    }
    log_info("DRM frame buffer map of %u bytes allocated at %p.\n",
             FrameBuffer_Size, DRM_FrameBuffer);

    drmModeFreeFB2(fb);
    drmModeFreeCrtc(crtc);
    drmModeFreeEncoder(encoder);
    drmModeFreeConnector(connector);
    drmModeFreeResources(ressource);
}
