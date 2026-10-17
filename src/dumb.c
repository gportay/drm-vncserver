#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm.h>

#define WIDTH  1920
#define HEIGHT 1200

int main(void)
{
    int drmfd = -1;
    int ret = EXIT_FAILURE;

    struct drm_mode_create_dumb creq = {0};
    struct drm_mode_map_dumb mreq = {0};
    struct drm_mode_destroy_dumb dreq = {0};

    uint32_t *pixels = MAP_FAILED;
    size_t size = 0;

    /* 1. Open DRM device */
    drmfd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    if (drmfd < 0) {
        perror("open /dev/dri/card0");
        return EXIT_FAILURE;
    }

    /* 2. Check dumb-buffer support */
    uint64_t has_dumb = 0;

    if (drmGetCap(drmfd, DRM_CAP_DUMB_BUFFER, &has_dumb) != 0) {
        perror("drmGetCap");
        goto cleanup;
    }

    if (!has_dumb) {
        fprintf(stderr, "Dumb buffers are not supported\n");
        goto cleanup;
    }

    /* 3. Allocate buffer */
    creq.width = WIDTH;
    creq.height = HEIGHT;
    creq.bpp = 32;

    if (drmIoctl(drmfd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) != 0) {
        perror("DRM_IOCTL_MODE_CREATE_DUMB");
        goto cleanup;
    }

    size = creq.size;

    printf("Handle : %u\n", creq.handle);
    printf("Pitch  : %u bytes\n", creq.pitch);
    printf("Size   : %zu bytes\n", size);

    /* 4. Obtain mmap offset */
    mreq.handle = creq.handle;

    if (drmIoctl(drmfd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) != 0) {
        perror("DRM_IOCTL_MODE_MAP_DUMB");
        goto cleanup;
    }

    /* 5. Map buffer */
    pixels = mmap(NULL,
                  size,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED,
                  drmfd,
                  mreq.offset);

    if (pixels == MAP_FAILED) {
        perror("mmap");
        goto cleanup;
    }

    /* 6. Draw a test pattern: XRGB8888 */
    for (unsigned int y = 0; y < HEIGHT; y++) {
        uint32_t *row =
            (uint32_t *)((uint8_t *)pixels + (size_t)y * creq.pitch);

        for (unsigned int x = 0; x < WIDTH; x++) {
            uint8_t r = (uint8_t)(x * 255 / WIDTH);
            uint8_t g = (uint8_t)(y * 255 / HEIGHT);
            uint8_t b = 128;

            row[x] = ((uint32_t)r << 16) |
                     ((uint32_t)g <<  8) |
                     ((uint32_t)b);
        }
    }

    /* 7. Save raw pixel data, excluding row padding */
    FILE *fp = fopen("framebuffer.raw", "wb");
    if (!fp) {
        perror("fopen");
        goto cleanup;
    }

    int write_error = 0;

    for (unsigned int y = 0; y < HEIGHT; y++) {
        const uint8_t *row =
            (const uint8_t *)pixels + (size_t)y * creq.pitch;

        if (fwrite(row, 1, WIDTH * 4, fp) != WIDTH * 4) {
            perror("fwrite");
            write_error = 1;
            break;
        }
    }

    if (fclose(fp) != 0) {
        perror("fclose");
        write_error = 1;
    }

    if (write_error)
        goto cleanup;

    printf("Saved framebuffer.raw (%u x %u, XRGB8888)\n",
           WIDTH, HEIGHT);

    ret = EXIT_SUCCESS;

cleanup:
    if (pixels != MAP_FAILED)
        munmap(pixels, size);

    if (creq.handle != 0) {
        dreq.handle = creq.handle;

        if (drmIoctl(drmfd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq) != 0)
            perror("DRM_IOCTL_MODE_DESTROY_DUMB");
    }

    if (drmfd >= 0)
        close(drmfd);

    return ret;
}
