#pragma once

int init_kbd(const char *device);
void cleanup_kbd();

void injectKeyEvent(uint16_t code, uint16_t value);
int keysym2scancode(rfbKeySym key);
