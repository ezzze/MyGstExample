# name
bin_PROGRAMS = tcloudPlayer

# list of source files
# the prefix is the name of the binary
tcloudPlayer_SOURCES = main.c gst-my.c fs-element-added-notifier.c

# list of headers we're not going to install
noinst_HEADERS = gst-my.h ../include/fs-element-added-notifier.h

# our CFLAGS and LDFLAGS used for compiling and linking
# make sure you prefix these with the name of your binary
tcloudPlayer_CFLAGS = -I $(top_srcdir)/include -Wall `pkg-config --cflags gtk+-3.0 gstreamer-video-1.0`
tcloudPlayer_LDFLAGS = `pkg-config --libs gtk+-3.0 gstreamer-video-1.0`
