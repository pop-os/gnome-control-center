
noinst_LTLIBRARIES = libgvc.la

INTROSPECTION_SCANNER_ARGS = --warn-all

libgvc_la_CPPFLAGS =			\
	$(WARN_CFLAGS)			\
	$(GVC_CFLAGS)			\
	-I$(srcdir)			\
	-DWITH_INTROSPECTION		\
	-DG_LOG_DOMAIN="\"Gvc\""

libgvc_la_gir_sources = 			\
	gvc-mixer-card.h			\
	gvc-mixer-card.c			\
	gvc-mixer-stream.h			\
	gvc-mixer-stream.c			\
	gvc-channel-map.h			\
	gvc-channel-map.c			\
	gvc-mixer-ui-device.h			\
	gvc-mixer-ui-device.c			\
	gvc-mixer-sink.h			\
	gvc-mixer-sink.c			\
	gvc-mixer-source.h			\
	gvc-mixer-source.c			\
	gvc-mixer-sink-input.h			\
	gvc-mixer-sink-input.c			\
	gvc-mixer-source-output.h		\
	gvc-mixer-source-output.c		\
	gvc-mixer-event-role.h			\
	gvc-mixer-event-role.c			\
	gvc-mixer-control.h			\
	gvc-mixer-control.c			\
	$(NULL)

libgvc_la_SOURCES =				\
	$(libgvc_la_gir_sources)		\
	gvc-mixer-card-private.h		\
	gvc-mixer-stream-private.h		\
	gvc-channel-map-private.h		\
	gvc-mixer-control-private.h		\
	gvc-pulseaudio-fake.h			\
	$(NULL)

libgvc_la_LIBADD = 	\
	$(GVC_LIBS)	\
	$(NULL)

if HAVE_INTROSPECTION
include $(INTROSPECTION_MAKEFILE)

Gvc-1.0.gir: libgvc.la
Gvc_1_0_gir_INCLUDES = GObject-2.0 Gio-2.0
Gvc_1_0_gir_CFLAGS = $(INCLUDES) -I$(srcdir) -DWITH_INTROSPECTION
Gvc_1_0_gir_LIBS = libgvc.la
Gvc_1_0_gir_FILES = $(addprefix $(srcdir)/,$(libgvc_la_gir_sources))
INTROSPECTION_GIRS = Gvc-1.0.gir

typelibdir = $(pkglibdir)
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = Gvc-1.0.gir $(typelib_DATA)

endif
