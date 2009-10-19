#include <cstring>
#include "display.h"
#include "block_helper.h"
#include "shaders.h"
#include "dumptga.h"

unsigned long display::rgb16_to_rgb32(unsigned long rgb16)
{
	if (rgb16 & 0x8000)
		return 0xFFFFFFF;
	unsigned long _r = rgb16 & 0x001F;
	unsigned long _g = rgb16 & 0x03E0;
	unsigned long _b = rgb16 & 0x7C00;
	return (_r << 3) | (_g << 6) | (_b << 9);
}

void display::pull_control()
{
	if (is_dirty(bregs))
		set_regs(pregs);
}
void display::pull_pal()
{
	if (is_dirty(bpal))
	{
		bg_pal.activate();
		bg_pal.resize(256);
		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, 
			GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, ppal );
	}
}

void display::init(unsigned long ioregs_addr, unsigned long pal_base,
				   unsigned long oam_base, unsigned long vram_base)
{
	unsigned long mask = NDSE::PAGE_SIZE - 1;
	bregs = NDSE::ARM9_GetPage(ioregs_addr);
	pregs = reinterpret_cast<graphic_registers*>(
		bregs->mem + (ioregs_addr & mask)
		);

	bpal = NDSE::ARM9_GetPage(pal_base);
	ppal = bpal->mem + (pal_base & mask);

	// really gotta clean this mess somewhat ...
	oam.init(oam_base, vram_base);
}


display::display(screen s)
{
	bg[0].init(s, BG_0);
	bg[1].init(s, BG_1);
	bg[2].init(s, BG_2);
	bg[3].init(s, BG_3);
}

void display::pull_all()
{
	pull_pal();
	for (int i = 0; i < 4; i++)
		bg[i].pull_data();
	oam.pull();
}

void display::set_regs(const graphic_registers *regs)
{
	dispcnt = regs->dispcnt;
	for (int i = 0; i < 4; i++)
	{
		bg[i].set_control( dispcnt, regs->bgcnt[i] );
		bg[i].set_offset( regs->bgofs[i] );
	}
	bg[2].set_affine( regs->bg2affine );
	bg[3].set_affine( regs->bg3affine );
	oam.set_control(dispcnt);
}


void display::prepare_shader(background &bg)
{
	if (bg.direct())
	{
		gl_program::finish();
		gl_program::glActiveTexture( GL_TEXTURE0 );
		bg.tex.activate();
		return;
	}

	shader *s = 0;
	switch (bg.get_mode())
	{
	case BG_CHAR8:
		s = shaders::char256; 
		break;
	case BG_PIXEL:
		s = shaders::plain256;
		break;
	}

	
	/*
	static int ctr = 0;
	ctr++;
	if ((ctr > 100) && (bg.get_mode() == BG_CHAR8))
	{
		dump1( bg_pal, "palette.tga" );
		dump2( bg.tex, "bg.tga" );
		dump2( bg.chartex, "charmap.tga" );
		exit(0);
	}*/
	

	// common attribs
	s->activate();
	
	s->glActiveTexture( GL_TEXTURE0 );
	s->uniform_i("palette", 0);
	bg_pal.activate();

	s->glActiveTexture( GL_TEXTURE1 );
	s->uniform_i("bg", 1);
	bg.tex.activate();

	// text mode only
	if (bg.get_mode() == BG_CHAR8)
	{
		s->glActiveTexture( GL_TEXTURE2 );
		s->uniform_i("charmap", 2);
		bg.chartex.activate();
	}
	s->glActiveTexture( GL_TEXTURE0 );
}


// optimize this (resize as needed and cache till not needed anymore)
struct r2t_formats
{
	render_to_texture r1024x1024;

	render_to_texture& get(int w, int h)
	{
		return r1024x1024;
	}

	r2t_formats() : r1024x1024(1024, 1024)
	{
	}
};

// clean this up!
// use initline and embed to class
void postprocess(int w, int h, int scalex, int scaley, shader *filter)
{
	static r2t_formats *r2t_fmt  = new r2t_formats();

	int ws = w * scalex;
	int hs = h * scaley;
	render_to_texture &rt = r2t_fmt->get(ws, hs);
	//double sw = (double)rt.tex.get_w()/ws;
	//double sh = (double)rt.tex.get_h()/hs;

	// render sprite/bg to a temporary texture
	rt.fbo.activate();
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport( 0, 0, ws, hs );
	glViewport(0,0,rt.tex.get_w(),rt.tex.get_h());
	glBegin(GL_QUADS);
	  glTexCoord2f(0.0f, 0.0f ); glVertex2f(-1.0f,-1.0f);
	  glTexCoord2f(1.0f, 0.0f ); glVertex2f( 1.0f,-1.0f);
	  glTexCoord2f(1.0f, 1.0f ); glVertex2f( 1.0f, 1.0f);
	  glTexCoord2f(0.0f, 1.0f ); glVertex2f(-1.0f, 1.0f);
	glEnd();
	glPopAttrib();
	gl_fbo::finish();

	// load texture to slot 0 and enable post filter
	filter->activate();
	filter->uniform_i("frame", 0);
	rt.tex.activate();
	//filter->uniform_2f("size", (float)ws, (float)hs);
	filter->uniform_2f("size", (float)rt.tex.get_w(),(float)rt.tex.get_h());
}

void display::do_display(screen scr, int w, int h)
{
	int h2 = h/2;
	int wn = w * 192;
	int hn = h2 * 256;
	
	int min = wn;
	if (hn < min)
		min = hn;


	// width  = 256
	// height = 192
	int w2 = min / 192;
	h2 = min / 256;

	double ratio = 0.75; // 192.0 / 256.0; 

	switch (scr)
	{
	case DISP_A: 
		//glViewport( (w-min)/2, h2, min, min ); 
		glViewport( (w-w2)/2, h/2, w2, h2 ); 
		break; // ., h-min, ., h2
	case DISP_B: 
		//glViewport( (w-min)/2, h2-min, min, min);
		glViewport( (w-w2)/2, h/2-h2, w2, h2 ); 
		break;
	}
	
	// clear screen
	glColor3f(0.2f,0.2f,0.2f);
	glBegin(GL_QUADS);
	  glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,-1.0f);
	  glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,-1.0f);
	  glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, 1.0f);
	  glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
	glEnd();
	glColor3f(1,1,1);

	glEnable(GL_TEXTURE_2D);

	pull_control();
	

	switch ((dispcnt >> 16) & 0x3)
	{
	case 1:
		pull_all();
		for (int i = 0; i < 4; i++)
		{
			background &bg = this->bg[i];
			if (!bg.enabled)
				continue;

			glBindTexture( GL_TEXTURE_2D, 0 );
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			
			/*
			glScaled((double)bg.tex.get_w()/bg.mode.w, 
					 (double)bg.tex.get_h()/bg.mode.h, 
					 1.0f);
			*/

			/*
			glScaled((double)256.0/bg.mode.w, 
					 (double)192.0/bg.mode.h, 
					 1.0);
			*/		 
			glMatrixMode(GL_MODELVIEW);
			
			prepare_shader(bg);
			//postprocess(bg.mode.w, bg.mode.h, 3.0, 3.0, shaders::post_filter );
			//postprocess(bg.mode.w, bg.mode.h, 3, 3, post_hq3x );
			//postprocess(bg.mode.w, bg.mode.h, 3, 3, post_rgb );

			bg.set_transform();
			

			

			//glEnable( GL_ALPHA_TEST );
			//glAlphaFunc( GL_GREATER, 0.5 );
			

			// for debugging purposes arrange all BGs in a rect
			/*
			glBegin(GL_QUADS);
			switch (i)
			{
			case 0: // topleft
				glTexCoord2f(0.0f,     scaler_y); glVertex2f(-1.0f, 0.0f);
				glTexCoord2f(scaler_x, scaler_y); glVertex2f( 0.0f, 0.0f);
				glTexCoord2f(scaler_x, 0.0f    ); glVertex2f( 0.0f, 1.0f);
				glTexCoord2f(0.0f,     0.0f    ); glVertex2f(-1.0f, 1.0f);
				break;
			case 1: // topright
				glTexCoord2f(0.0f,     scaler_y); glVertex2f( 0.0f, 0.0f);
				glTexCoord2f(scaler_x, scaler_y); glVertex2f( 1.0f, 0.0f);
				glTexCoord2f(scaler_x, 0.0f    ); glVertex2f( 1.0f, 1.0f);
				glTexCoord2f(0.0f,     0.0f    ); glVertex2f( 0.0f, 1.0f);
				break;
			case 2: // bottomleft
				glTexCoord2f(0.0f,     scaler_y); glVertex2f(-1.0f,-1.0f);
				glTexCoord2f(scaler_x, scaler_y); glVertex2f( 0.0f,-1.0f);
				glTexCoord2f(scaler_x, 0.0f    ); glVertex2f( 0.0f, 0.0f);
				glTexCoord2f(0.0f,     0.0f    ); glVertex2f(-1.0f, 0.0f);
				break;
			case 3: // bottomright
				glTexCoord2f(0.0f,     scaler_y); glVertex2f( 0.0f,-1.0f);
				glTexCoord2f(scaler_x, scaler_y); glVertex2f( 1.0f,-1.0f);
				glTexCoord2f(scaler_x, 0.0f    ); glVertex2f( 1.0f, 0.0f);
				glTexCoord2f(0.0f,     0.0f    ); glVertex2f( 0.0f, 0.0f);
				break;
			}  
			glEnd();
			*/

			glMatrixMode(GL_TEXTURE);
			double afx = 0.0*bg.affine[0] / bg.mode.w;
			double afy = 0.0*bg.affine[1] / bg.mode.h;
			glBegin(GL_QUADS);
			glTexCoord2d(0.0f-afx, 1.0f-afy); glVertex2f(-1.0f,-1.0f);
			glTexCoord2d(1.0f-afx, 1.0f-afy); glVertex2f( 1.0f,-1.0f);
			glTexCoord2d(1.0f-afx, 0.0f-afy); glVertex2f( 1.0f, 1.0f);
			glTexCoord2d(0.0f-afx, 0.0f-afy); glVertex2f(-1.0f, 1.0f);
			glEnd();
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			

			gl_program::finish();
		}
		break;
	case 2: // VRAM display
		// pull vram as raw texture to bg0

		// pull the requested bank
		bg[0].tex.resize(256, 256);
		bg[0].pull_direct( NDSE::MEM_GetVRAM( (dispcnt >> 18)&3 )->start );
		

		gl_program::glActiveTexture( GL_TEXTURE0 );
		bg[0].tex.activate();

		glBegin(GL_QUADS);
		glTexCoord2d(0.0f, 0.75f); glVertex2f(-1.0f,-1.0f);
		glTexCoord2d(1.0f, 0.75f); glVertex2f( 1.0f,-1.0f);
		glTexCoord2d(1.0f, 0.0f); glVertex2f( 1.0f, 1.0f);
		glTexCoord2d(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
		glEnd();

		break;
	}
	glDisable(GL_TEXTURE_2D);

	oam.display();
	
	GLenum err = glGetError();
	if (err != 0)
		std::cerr << "GL error: " << err << std::endl;
	//assert( glGetError() == 0 );
}

