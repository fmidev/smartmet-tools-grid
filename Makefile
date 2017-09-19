SUBNAME = grid
LIB = smartmet-$(SUBNAME)
SPEC = smartmet-library-$(SUBNAME)
INCDIR = smartmet/$(SUBNAME)

# Enabling / disabling CORBA usage.

CORBA = enabled


# Installation directories

processor := $(shell uname -p)

ifeq ($(origin PREFIX), undefined)
  PREFIX = /usr
else
  PREFIX = $(PREFIX)
endif

ifeq ($(processor), x86_64)
  libdir = $(PREFIX)/lib64
else
  libdir = $(PREFIX)/lib
endif

bindir = $(PREFIX)/bin
includedir = $(PREFIX)/include
datadir = $(PREFIX)/share
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

ifeq ($(CXX), clang++)

 FLAGS = \
	-std=c++11 -fPIC -MD \
	-Weverything \
	-Wno-c++98-compat \
	-Wno-float-equal \
	-Wno-padded \
	-Wno-missing-prototypes

 INCLUDES = \
	-isystem $(includedir) \
	-isystem $(includedir)/smartmet

else

 FLAGS = -std=c++11 -fdiagnostics-color=always -fPIC -MD -Wall -W -Wno-unused-parameter

 FLAGS_DEBUG = \
	-Wcast-align \
	-Winline \
	-Wno-multichar \
	-Wno-pmf-conversions \
	-Woverloaded-virtual  \
	-Wpointer-arith \
	-Wcast-qual \
	-Wredundant-decls \
	-Wwrite-strings \
	-Wno-sign-promo \
	-Wno-unknown-pragmas \
	-Wno-inline

 FLAGS_RELEASE = -Wuninitialized

 INCLUDES = \
	-I$(includedir) \
	-I$(includedir)/smartmet \
	-I$(includedir)/smartmet/grid-files \
	-I$(includedir)/smartmet/grid-content \
	$(pkg-config --cflags icu-i18n) \
	$(CORBA_INCLUDE)

endif

# Compile options in detault, debug and profile modes

CFLAGS         = $(DEFINES) $(FLAGS) $(FLAGS_RELEASE) -DNDEBUG -O2 -g
CFLAGS_DEBUG   = $(DEFINES) $(FLAGS) $(FLAGS_DEBUG)   -Werror  -Og -g
CFLAGS_PROFILE = $(DEFINES) $(FLAGS) $(FLAGS_PROFILE) -DNDEBUG -O2 -g -pg


LIBS = -L$(libdir) \
	-lsmartmet-spine \
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
	-lgdal \
	-lopenjpeg \
	-ljpeg \
	-lpng \
	-lhiredis \
	-lmicrohttpd \
	-lcurl \
	-lpq \
	$(CORBA_LIBS)

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
PROGS    = $(SRCS:%.cpp=%)
OBJS     = $(SRCS:%.cpp=%.o)
OBJFILES = $(OBJS:%.o=obj/%.o)

INCLUDES := -Isrc $(INCLUDES)


.PHONY: test rpm

# The rules

all: objdir $(PROGS)

debug: objdir $(OBJFILES) $(PROGS)

#debug: all

release: all

profile: all

src_debug: objdir $(LIBFILE)


$(PROGS): % : %.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o bin/$@ obj/$@.o $(LIBS)


clean: 
	rm -f src/*~ src/*/*~ src/*/*/*~
	rm -rf obj
	rm -rf bin/*

format:
	clang-format -i -style=file $(SUBNAME)/*.h $(SUBNAME)/*.cpp test/*.cpp

install:

test:
	+cd test && make test

objdir:
	@mkdir -p obj
	@mkdir -p obj/clients
	@mkdir -p obj/files
	@mkdir -p obj/fmi
	@mkdir -p obj/servers
	@mkdir -p obj/utils
	@mkdir -p bin
	@mkdir -p bin/clients
	@mkdir -p bin/files
	@mkdir -p bin/fmi
	@mkdir -p bin/servers
	@mkdir -p bin/utils

rpm: clean
	if [ -e $(SPEC).spec ]; \
	then \
	  tar -czvf $(SPEC).tar.gz --transform "s,^,$(SPEC)/," * ; \
	  rpmbuild -ta $(SPEC).tar.gz ; \
	  rm -f $(SPEC).tar.gz ; \
	else \
	  echo $(SPEC).spec file missing; \
	fi;

.SUFFIXES: $(SUFFIXES) .cpp


obj/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

ifneq ($(wildcard obj/*.d),)
-include $(wildcard obj/*.d)
endif
