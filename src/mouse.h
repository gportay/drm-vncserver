#pragma once

int init_mouse(const char *device, int vnc_rotate);
void cleanup_mouse();
void injectMouseEvent(struct fb_var_screeninfo *scrinfo, int buttonMask, int x, int y);
