#pragma once

int init_mouse(const char *mouse_device, int vnc_rotate);
void cleanup_mouse();
void injectMouseEvent(int buttonMask, int x, int y, int xres, int yres);
