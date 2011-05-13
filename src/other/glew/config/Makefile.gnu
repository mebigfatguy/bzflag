NAME = $(GLEW_NAME)
CC = cc
LD = cc
ifneq (undefined, $(origin GLEW_MX))
CFLAGS.EXTRA = -DGLEW_MX
endif
PICFLAG = -fPIC
LDFLAGS.SO = -shared -Wl,-soname=$(LIB.SONAME)
LDFLAGS.EXTRA = -L/usr/X11R6/lib
LDFLAGS.GL = -lXmu -lXi -lGLU -lGL -lXext -lX11
LDFLAGS.STATIC = -Wl,-Bstatic
LDFLAGS.DYNAMIC = -Wl,-Bdynamic
NAME = GLEW
WARN = -Wall -W
POPT = -O2
BIN.SUFFIX =
LIB.SONAME = lib$(NAME).so.$(SO_MAJOR)
LIB.DEVLNK = lib$(NAME).so
LIB.SHARED = lib$(NAME).so.$(SO_VERSION)
LIB.STATIC = lib$(NAME).a
SHARED_OBJ_EXT = pic_o
