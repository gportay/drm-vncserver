#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <sys/sysmacros.h> /* For makedev() */

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>

#include <assert.h>
#include <errno.h>

/* libvncserver */
#include <rfb/rfb.h>

#include "touch.h"
#include "tklog.h"

static int fd = -1;

static int xmin, xmax;
static int ymin, ymax;
static int rotate;
static int trkg_id = -1;

#ifndef input_event_sec
#define input_event_sec time.tv_sec
#define input_event_usec time.tv_usec
#endif

int init_touch(const char *device, int vnc_rotate)
{
    struct input_absinfo info;

    tklog_info("Initializing touch device %s ...\n", device);
    if ((fd = open(device, O_RDWR)) == -1) {
        tklog_error("cannot open touch device %s\n", device);
        return 0;
    }

    /* Get the Range of X and Y */
    if (ioctl(fd, EVIOCGABS(ABS_X), &info)) {
        tklog_error("cannot get ABS_X info, %s\n", strerror(errno));
        return 0;
    }
    xmin = info.minimum;
    xmax = info.maximum;
    if (ioctl(fd, EVIOCGABS(ABS_Y), &info)) {
        tklog_error("cannot get ABS_Y, %s\n", strerror(errno));
        return 0;
    }
    ymin = info.minimum;
    ymax = info.maximum;
    rotate = vnc_rotate;

    tklog_info("  x:(%d %d)  y:(%d %d) \n", xmin, xmax, ymin, ymax);
    return 1;
}

void cleanup_touch()
{
    if (fd != -1)
        close(fd);
}

void injectTouchEvent(enum MouseAction mouseAction, int x, int y,
                      struct fb_var_screeninfo *scrinfo)
{
    /*
     * Indicates either pointer movement or a pointer button press or release.
     * The pointer is now at (x-position, y-position), and the current state of
     * buttons 1 to 8 are represented by bits 0 to 7 of button-mask
     * respectively, 0 meaning up, 1 meaning down (pressed).  On a conventional
     * mouse, buttons 1, 2 and 3 correspond to the left, middle and right
     * buttons on the mouse. On a wheel mouse, each step of the wheel upwards
     * is represented by a press and release of button 4, and each step
     * downwards is represented by a press and release of button 5.
     *
     * From: http://www.vislab.usyd.edu.au/blogs/index.php/2009/05/22/an-headerless-indexed-protocol-for-input-1?blog=61
     */
    struct input_event ev;
    int xin = x;
    int yin = y;

    switch (rotate) {
    case 90:
        x = yin;
        y = scrinfo->yres - 1 - xin;
        break;
    case 180:
        x = scrinfo->xres - 1 - xin;
        y = scrinfo->yres - 1 - yin;
        break;
    case 270:
        x = scrinfo->xres - 1 - yin;
        y = xin;
        break;
    }

    /* Calculate the final x and y */
    x = xmin + (x * (xmax - xmin)) / (scrinfo->xres);
    y = ymin + (y * (ymax - ymin)) / (scrinfo->yres);

    memset(&ev, 0, sizeof(ev));

    bool sendPos;
    bool sendTouch;
    int trkIdValue;
    int touchValue;
    struct timeval time;

    switch (mouseAction) {
    case MousePress:
        sendPos = true;
        sendTouch = true;
        trkIdValue = ++trkg_id;
        touchValue = 1;
        break;
    case MouseRelease:
        sendPos = false;
        sendTouch = true;
        trkIdValue = -1;
        touchValue = 0;
        break;
    case MouseDrag:
        sendPos = true;
        sendTouch = false;
        break;
    default:
        tklog_error("invalid mouse action\n");
        exit(EXIT_FAILURE);
    }

    if (sendTouch) {
        /* Then send a ABS_MT_TRACKING_ID */
        gettimeofday(&time, 0);
        ev.input_event_sec = time.tv_sec;

        ev.type = EV_ABS;
        ev.code = ABS_MT_TRACKING_ID;
        ev.value = trkIdValue;
        if (write(fd, &ev, sizeof(ev)) < 0)
            tklog_error("write event failed, %s\n", strerror(errno));

        /* Then send a BTN_TOUCH */
        gettimeofday(&time, 0);
        ev.input_event_sec = time.tv_sec;
        ev.input_event_usec = time.tv_usec;
        ev.type = EV_KEY;
        ev.code = BTN_TOUCH;
        ev.value = touchValue;
        if (write(fd, &ev, sizeof(ev)) < 0)
            tklog_error("write event failed, %s\n", strerror(errno));
    }

    if (sendPos) {
        /* Then send a ABS_MT_POSITION_X */
        gettimeofday(&time, 0);
        ev.input_event_sec = time.tv_sec;
        ev.input_event_usec = time.tv_usec;
        ev.type = EV_ABS;
        ev.code = ABS_MT_POSITION_X;
        ev.value = x;
        if (write(fd, &ev, sizeof(ev)) < 0)
            tklog_error("write event failed, %s\n", strerror(errno));

        /* Then send a ABS_MT_POSITION_Y */
        gettimeofday(&time, 0);
        ev.input_event_sec = time.tv_sec;
        ev.input_event_usec = time.tv_usec;
        ev.type = EV_ABS;
        ev.code = ABS_MT_POSITION_Y;
        ev.value = y;
        if (write(fd, &ev, sizeof(ev)) < 0)
            tklog_error("write event failed, %s\n", strerror(errno));

        /* Then send the X */
        gettimeofday(&time, 0);
        ev.input_event_sec = time.tv_sec;
        ev.input_event_usec = time.tv_usec;
        ev.type = EV_ABS;
        ev.code = ABS_X;
        ev.value = x;
        if (write(fd, &ev, sizeof(ev)) < 0)
            tklog_error("write event failed, %s\n", strerror(errno));

        /* Then send the Y */
        gettimeofday(&time, 0);
        ev.input_event_sec = time.tv_sec;
        ev.input_event_usec = time.tv_usec;
        ev.type = EV_ABS;
        ev.code = ABS_Y;
        ev.value = y;
        if (write(fd, &ev, sizeof(ev)) < 0)
            tklog_error("write event failed, %s\n", strerror(errno));
    }

    /* Finally send the SYN */
    gettimeofday(&time, 0);
    ev.input_event_sec = time.tv_sec;
    ev.input_event_usec = time.tv_usec;
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if (write(fd, &ev, sizeof(ev)) < 0)
        tklog_error("write event failed, %s\n", strerror(errno));

    tklog_debug("injectTouchEvent (screen(%d,%d) -> touch(%d,%d), mouse=%d)\n",
                xin, yin, x, y, mouseAction);
}
