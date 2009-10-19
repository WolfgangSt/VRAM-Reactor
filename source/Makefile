CC =		g++
AR =		ar
#ARFLAGS =	@ARFLAGS@
INCLUDES =	-I. -I../NDSE-SDK/include 
CFLAGS =	-fPIC -g -O2 -fvisibility=hidden $(INCLUDES)
LDFLAGS =

SRCS=block_helper.cpp hq3x.cpp shader.cpp shaders.cpp \
	VRAM_Reactor.cpp


OBJS	:=	$(SRCS:.cpp=.o)

all: libVRAM_Reactor.a libVRAM_Reactor.so

$(OBJS): %.o:
	$(CC) $(CFLAGS) -c $*.cpp -o $@


libVRAM_Reactor.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

libVRAM_Reactor.so: $(OBJS)
	$(CC) -shared -Wl,-soname=libVRAM_Reactor.so $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o
	rm -f libVRAM_Reactor.a
	rm -f libVRAM_Reactor.so* 





