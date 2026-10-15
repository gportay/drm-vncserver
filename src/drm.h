#pragma once

uint32_t *DRM_FrameBuffer = MAP_FAILED;

unsigned int FrameBuffer_BitsPerPixel;
unsigned int FrameBuffer_Depth;
unsigned int FrameBuffer_Size;
unsigned int FrameBuffer_Xwidth;
unsigned int FrameBuffer_Yheight;
unsigned int FrameBuffer_RedMax;
unsigned int FrameBuffer_GreenMax;
unsigned int FrameBuffer_BlueMax;
unsigned int FrameBuffer_RedShift;
unsigned int FrameBuffer_GreenShift;
unsigned int FrameBuffer_BlueShift;

int init_drm(const char *device);
void cleanup_drm();
