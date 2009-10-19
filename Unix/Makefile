CC =		g++
AR =		ar
#ARFLAGS =	@ARFLAGS@
QT_ROOT = /opt/qtsdk-2009.02/qt
QT_INC = $(QT_ROOT)/include
QT_LIB = $(QT_ROOT)/lib
INCLUDES =	-I. -I../../include  -I$(QT_INC) -I$(QT_INC)/QtCore
CFLAGS =	-fPIC -g -O2 -fvisibility=hidden $(INCLUDES)
LIBS = -lQtCore
LIBDIRS = -L$(QT_LIB)
LDFLAGS = $(LIBDIRS) $(LIBS)

SRCS=block_helper.cpp hq3x.cpp shader.cpp shaders.cpp \
	VRAM_Reactor.cpp
#OBJS	:=	$(addprefix Debug/,$(notdir $(SRCS:.cpp=.o)))
OBJS	:=	$(addprefix Debug/,$(SRCS:.cpp=.o))

all: Debug/libVRAM_Reactor.a Debug/libVRAM_Reactor.so


#OUTFILE = $(addprefix Debug/,$(notdir $(src:.cpp=.o)))
#CCLINE = echo $(CC) $(CFLAGS) -c $(src) -o $(OUTFILE)
#BOJBS=$(foreach src,$(SRCS),$(OUTFILE): $(src)"\n"$(CCLINE)"\n\n")
#$(BOBJS)

#$(OBJS): $(SRCS)
#	#-mkdir Debug
#	echo $(CC) $(CFLAGS) -c $@.cpp -o $@

$(OBJS): %.o:
	$(CC) $(CFLAGS) -c $(subst Debug/,,$*.cpp) -o $@


Debug/libVRAM_Reactor.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

Debug/libVRAM_Reactor.so: $(OBJS)
	$(CC) -shared -Wl,-soname=libVRAM_Reactor.so $(LDFLAGS) -o $@ $(OBJS)

clean:
	-rm Debug/*.o

test:
	echo $(BOJBS)





