SUBNAME = grid
LIB = smartmet-$(SUBNAME)
SPEC = smartmet-tools-$(SUBNAME)
INCDIR = smartmet/$(SUBNAME)

# Enabling / disabling CORBA usage.

CORBA = enabled

REQUIRES = libpqxx gdal

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc


# Installation directories

processor := $(shell uname -p)

ifeq ($(origin PREFIX), undefined)
  EPREFIX = /usr
else
  EPREFIX = $(PREFIX)
endif

ifeq ($(processor), x86_64)
  libdir = $(EPREFIX)/lib64
else
  libdir = $(EPREFIX)/lib
endif

bindir = $(EPREFIX)/bin
includedir = $(EPREFIX)/include
datadir = $(EPREFIX)/share
objdir = obj


ifeq ($(CORBA), disabled)
  CORBA_FLAGS = -DCORBA_DISABLED
else
  CORBA_INCLUDE = -I/usr/include/smartmet/grid-content/contentServer/corba/stubs \
                  -I/usr/include/smartmet/grid-content/dataServer/corba/stubs \
                  -I/usr/include/smartmet/grid-content/queryServer/corba/stubs
  CORBA_LIBS = -lomniORB4 -lomnithread
endif



# Compiler options

DEFINES = -DUNIX -D_REENTRANT

# Boost 1.69

ifneq "$(wildcard /usr/include/boost169)" ""
  INCLUDES += -isystem /usr/include/boost169
  LIBS += -L/usr/lib64/boost169
endif

ifneq "$(wildcard /usr/gdal32/include)" ""
  INCLUDES += -isystem /usr/gdal32/include
  LIBS += -L$(PREFIX)/gdal32/lib -lgdal
else
  ifneq "$(wildcard /usr/gdal30/include)" ""
    INCLUDES += -isystem /usr/gdal30/include
    LIBS += -L$(PREFIX)/gdal30/lib -lgdal
  else
    INCLUDES += -isystem /usr/include/gdal
    LIBS += -lgdal
  endif
endif


FLAGS = -std=c++11 -fdiagnostics-color=always -fPIC -MD -Wall -W -Wno-unused-parameter

FLAGS_DEBUG = \
	-Wcast-align \
	-Winline \
	-Wno-multichar \
	-Wno-pmf-conversions \
	-Wpointer-arith \
	-Wcast-qual \
	-Wredundant-decls \
	-Wwrite-strings \
	-Wno-sign-promo \
	-Wno-unknown-pragmas \
	-Wno-inline

# Disabled for now:
#	-Woverloaded-virtual

FLAGS_RELEASE = -Wuninitialized

INCLUDES += \
	-I$(includedir)/smartmet \
	`pkg-config --cflags freetype2` \
	`pkg-config --cflags icu-i18n` \
	$(CORBA_INCLUDE)

# Compile options in detault, debug and profile modes

CFLAGS         = $(DEFINES) $(FLAGS) $(FLAGS_RELEASE) -DNDEBUG -O2 -g
CFLAGS_DEBUG   = $(DEFINES) $(FLAGS) $(FLAGS_DEBUG)   -Werror  -Og -g
CFLAGS_PROFILE = $(DEFINES) $(FLAGS) $(FLAGS_PROFILE) -DNDEBUG -O2 -g -pg


LIBS += -L$(libdir) \
	$(REQUIRED_LIBS) \
	-lsmartmet-spine \
	-lsmartmet-macgyver \
	-lsmartmet-newbase \
	-lsmartmet-grid-files \
	-lsmartmet-grid-content \
	-lboost_program_options \
	-lboost_filesystem \
	-lboost_regex \
	-lboost_thread \
	-lboost_system \
	-lboost_iostreams \
	-lboost_date_time \
	-lpthread \
	-ljpeg \
	-lpng \
	-lhiredis \
	-lmicrohttpd \
	-lcurl \
	$(CORBA_LIBS) \
	-lssl -lcrypto \
	-lstdc++ -lm \
	-L /usr/lib/x86_64-linux-gnu -lpq \
	`pkg-config --libs-only-l freetype2`

# What to install

LIBFILE = libsmartmet-$(SUBNAME).so

# How to install

INSTALL_PROG = install -p -m 775
INSTALL_DATA = install -p -m 664

# Compile option overrides

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_DEBUG)
endif

ifneq (,$(findstring profile,$(MAKECMDGOALS)))
  CFLAGS = $(CFLAGS_PROFILE)
endif

# Compilation directories

vpath %.cpp src \
			src/clients \
			src/files \
			src/fmi \
			src/servers \
			src/utils

vpath %.h 	src \
			src/clients \
			src/files \
			src/fmi \
			src/servers \
			src/utils

vpath %.o obj



# The files to be compiled


SRCS     = $(patsubst src/%,%,$(wildcard src/*.cpp src/*/*.cpp))
PROGS    = $(SRCS:%.cpp=bin/%)
OBJS     = $(SRCS:%.cpp=obj/bin/%.o)

INCLUDES := -Isrc $(INCLUDES)


.PHONY: test rpm

# The rules

all: objdir $(PROGS)

debug: objdir $(OBJS) $(PROGS)

#debug: all

release: all

profile: all

src_debug: objdir $(LIBFILE)


$(PROGS): % : obj/%.o
	$(CC) $(LDFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ obj/$@.o $(LIBS)


clean:
	rm -f src/*~ src/*/*~ src/*/*/*~
	rm -rf obj
	rm -rf bin/*

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp test/*.cpp

install:
	mkdir -p $(bindir)/fmi
	mkdir -p $(bindir)/servers
	mkdir -p $(bindir)/utils
	mkdir -p $(bindir)/files
	mkdir -p $(bindir)/clients
	@list='$(PROGS)'; \
	for prog in $$list; do \
	  echo $(INSTALL_PROG) $$prog $(bindir)/../$$prog; \
	  $(INSTALL_PROG) $$prog $(bindir)/../$$prog; \
	done
	mkdir -p $(libdir)/../lib/systemd/system
	$(INSTALL_DATA) systemd/radon2smartmet.service  $(libdir)/../lib/systemd/system
test:
	+cd test && make test

objdir:
	@mkdir -p obj/bin/clients
	@mkdir -p obj/bin/files
	@mkdir -p obj/bin/fmi
	@mkdir -p obj/bin/servers
	@mkdir -p obj/bin/utils
	@mkdir -p bin/clients
	@mkdir -p bin/files
	@mkdir -p bin/fmi
	@mkdir -p bin/servers
	@mkdir -p bin/utils

rpm: $(SPEC).spec
	rm -f $(SPEC).tar.gz # Clean a possible leftover from previous attempt
	tar -czvf $(SPEC).tar.gz --exclude bin --exclude-vcs --transform "s,^,$(SPEC)/," *
	rpmbuild -tb $(SPEC).tar.gz
	rm -f $(SPEC).tar.gz

.SUFFIXES: $(SUFFIXES) .cpp


obj/bin/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

-include obj/*/*.d
