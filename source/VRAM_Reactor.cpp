// VRAM_Reactor.cpp : Defines the entry point for the console application.
//

// use glColorTable (1.2 core feature) rather than texture lookups for paletting?
// => just allows one concurrent palette though

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>


#include <assert.h>
#include <iostream>
#include <fstream>

#include "shader.h"
#include "ioregs.h"
#include "fixedpoint.h"
#include "hq3x.h"
#include "block_helper.h"

#include "dumptga.h"

#include <stdio.h>
#include <string.h>


#include "video/video.h"
#include "shaders.h"

#define EXPORT
#include "VRAM_Reactor.h"

// FIXME
// This aint nice: rather pull this to a better makefile
#include "video/video.cpp"
#include "bg.h"
#include "display.h"

GetExtensionCB fpGetExtension = 0;

class display_a: public display
{
public:
	void init()
	{
		display::init(0x04000000, 0x05000000, 0x07000000, 0x06400000);
	}
	display_a() : display(DISP_A) {}
} disp_a;

class display_b: public display
{
public:
	void init()
	{
		display::init(0x04001000, 0x05000400, 0x07000400, 0x06600000);
	}
	display_b() : display(DISP_B) {}
} disp_b;




void init_gl()
{
	gl_shader::init();
	glClearColor(0.7f,0.7f,0.7f, 0.0f);
}

void* GetExtension(const char *name)
{
	if (!fpGetExtension)
		return 0;
	return fpGetExtension(name);
}


void STDCALL VideoInit2(GetExtensionCB cb)
{
	fpGetExtension = cb;
	init_gl();
	shaders::init();
	disp_a.init();
	disp_b.init();
}

void STDCALL Redisplay(int w, int h)
{
	// no synco at all yet
	// fire vcount and vblank at end of frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	disp_a.do_display(DISP_A, w, h);
	disp_b.do_display(DISP_B, w, h);
	flush_now();
	
	NDSE::ARM7_Interrupt(2); // vcount
	NDSE::ARM7_Interrupt(0); // vblank

	NDSE::ARM9_Interrupt(2); // vcount
	NDSE::ARM9_Interrupt(0); // vblank
}
