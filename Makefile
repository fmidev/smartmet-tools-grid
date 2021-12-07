SUBNAME = grid
LIB = smartmet-$(SUBNAME)
SPEC = smartmet-tools-$(SUBNAME)
INCDIR = smartmet/$(SUBNAME)

# Enabling / disabling CORBA usage.

CORBA = enabled

REQUIRES = libpqxx gdal icu-i18n

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

ifneq ($(wildcard /usr/pgsql-13/lib/libpq.so),)
INCLUDES += -isystem /usr/pgsql-13/include
REQUIRED_LIBS += -L/usr/pgsql-13/lib
endif

ifeq ($(CORBA), disabled)
  CORBA_FLAGS = -DCORBA_DISABLED
else
  CORBA_INCLUDE = -I/usr/include/smartmet/grid-content/contentServer/corba/stubs \
                  -I/usr/include/smartmet/grid-content/dataServer/corba/stubs \
                  -I/usr/include/smartmet/grid-content/queryServer/corba/stubs
  CORBA_LIBS = -lomniORB4 -lomnithread
endif

DEFINES = -DUNIX -D_REENTRANT

INCLUDES += \
	-I$(includedir)/smartmet \
	`pkg-config --cflags freetype2` \
	`pkg-config --cflags icu-i18n` \
	$(CORBA_INCLUDE)

# Compile options in detault, debug and profile modes
LIBS += -L$(libdir) \
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
	$(REQUIRED_LIBS) \
	-lssl -lcrypto \
	-lstdc++ -lm \
	-L /usr/lib/x86_64-linux-gnu -lpq \
	`pkg-config --libs-only-l freetype2`

# What to install

LIBFILE = libsmartmet-$(SUBNAME).so

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
	$(CXX) $(LDFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ obj/$@.o $(LIBS)


clean:
	rm -f src/*~ src/*/*~ src/*/*/*~
	rm -rf obj
	rm -rf bin/*

clean-install:
	rm -rf $(bindir)/fmi
	rm -rf $(bindir)/servers
	rm -rf $(bindir)/utils
	rm -rf $(bindir)/files
	rm -rf $(bindir)/clients

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
