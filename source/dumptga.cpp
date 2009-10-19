#include "dumptga.h"
#include <GL/glext.h>
#include <stdio.h>
#include <string.h>

#include <pshpack1.h>
	struct tga_header
	{
		unsigned char ident;
		unsigned char has_pal;
		unsigned char type;
		unsigned short pal_start;
		unsigned short pal_len;
		unsigned char pal_bits;
		unsigned short xoff;
		unsigned short yoff;
		unsigned short w;
		unsigned short h;
		unsigned char bits;
		unsigned char flags;
	} hdr;
#include <poppack.h>

void dump2(texture2d &tex, char *name)
{
	memset( &hdr, 0, sizeof(hdr) );
	hdr.type = 2; // RGB
	hdr.w = (unsigned short)tex.get_w();
	hdr.h = (unsigned short)tex.get_h();
	hdr.bits = 32;

	// render offscreen
	gl_program::finish();
	render_to_texture rt( tex.get_w(), tex.get_h() );
	rt.fbo.activate();
	glPushAttrib(-1);
	glViewport(0,0,rt.tex.get_w(), rt.tex.get_h());
	
	gl_program::glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	gl_program::glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	gl_program::glActiveTexture( GL_TEXTURE0 );
	tex.activate();
	glDisable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	  glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f,-1.0f);
	  glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f,-1.0f);
	  glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f, 1.0f);
	  glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
	glEnd();
	
	glPopAttrib();
	glFlush();

	// read back
	unsigned long *pixels = new unsigned long[tex.get_w() * tex.get_h()];
	glReadPixels( 0,0,tex.get_w(), tex.get_h(), GL_BGRA, GL_UNSIGNED_BYTE, pixels );
	rt.fbo.finish();

	FILE *f;
	f = fopen(name, "wb");
	fwrite( &hdr, 1, sizeof(hdr), f );
	fwrite( pixels, sizeof(*pixels), tex.get_w() * tex.get_h(), f );
	delete []pixels;
	fclose(f);
}

void dump1(texture1d &tex, char *name)
{
	memset( &hdr, 0, sizeof(hdr) );
	hdr.type = 2; // RGB
	hdr.w = (unsigned short)tex.get_w();
	hdr.h = 64;
	hdr.bits = 32;

	// render offscreen
	gl_program::finish();
	render_to_texture rt( tex.get_w(), 64 );
	rt.fbo.activate();
	glPushAttrib(-1);
	glViewport(0,0,rt.tex.get_w(), rt.tex.get_h());
	gl_program::glActiveTexture( GL_TEXTURE0 );
	tex.activate();
	glEnable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	  glTexCoord1f(0.0f); glVertex2f(-1.0f,-1.0f);
	  glTexCoord1f(1.0f); glVertex2f( 1.0f,-1.0f);
	  glTexCoord1f(1.0f); glVertex2f( 1.0f, 1.0f);
	  glTexCoord1f(0.0f); glVertex2f(-1.0f, 1.0f);
	glEnd();
	glPopAttrib();
	glFlush();

	// read back
	unsigned long *pixels = new unsigned long[tex.get_w() * 64];
	glReadPixels( 0,0,tex.get_w(), 64, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
	rt.fbo.finish();

	FILE *f;
	f = fopen(name, "wb");
	fwrite( &hdr, 1, sizeof(hdr), f );
	fwrite( pixels, sizeof(*pixels), tex.get_w() * 64, f );
	delete []pixels;
	fclose(f);
}
