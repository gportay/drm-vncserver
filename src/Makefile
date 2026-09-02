PREFIX ?= /usr/local
CFLAGS += -g -Wextra -Wall

.PHONY: all
all: drm-vncserver

drm-vncserver: LOADLIBES += $(shell pkg-config libdrm libvncserver --libs)
drm-vncserver: drm-vncserver.o
drm-vncserver: keyboard.o
drm-vncserver: mouse.o
drm-vncserver: touch.o

drm-vncserver.o: CFLAGS += $(shell pkg-config libdrm --cflags)
drm-vncserver.o: drm-vncserver.c
drm-vncserver.o: keyboard.h
drm-vncserver.o: mouse.h
drm-vncserver.o: touch.h
drm-vncserver.o: tklog.h

keyboard.o: keyboard.c
keyboard.o: keyboard.h
keyboard.o: tklog.h

mouse.o: mouse.c
mouse.o: mouse.h
mouse.o: tklog.h

touch.o: touch.c
touch.o: touch.h
touch.o: tklog.h

.PHONY: install
install:
	install -D -m0755 drm-vncserver $(DESTDIR)$(PREFIX)/bin/drm-vncserver

.PHONY: clean
clean:
	rm -f drm-vncserver *.o
