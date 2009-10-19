#include "bg.h"
#include "block_helper.h"
#include "fixedpoint.h"
#include "charhelp.h"

#include <assert.h>
#include <GL/glext.h>

tile_mode background::get_mode()
{ 
	return tilemode; 
}

void background::set_transform()
{
	//double scaler_x = 256.0/mode.w;
	//double scaler_y = 256.0/mode.h;
	double scaler_x = 1.0f;
	double scaler_y = 1.0f;

	if (get_mode() == BG_CHAR8 )
	{
		scaler_x *= 0.25;
		scaler_y *= 0.25;
	}

	double afx = affine[0];
	double afy = affine[1];

	scaler_y *= 0.75;
	glMatrixMode(GL_TEXTURE);		

	//glTranslated( afx/mode.w + 0.5, afy/mode.h +0.5, 0.0 );
	//glTranslated( 0.5, 0.5, 0.0 );
	
	/*
	// BUGS fire demo
	if ((bg != BG_0) && (bg != BG_1))
	{
		
		glMultMatrixd(mat);
		//glTranslated(0.5, 0.5, 0.0);
	}
	*/
	//glTranslated( -0.5, -0.5, 0.0 );
	//glTranslated( -((afx+offset[0])/mode.w) -0.5, -((afy+offset[1])/mode.h) -0.5, 0.0 );
	
	
	glTranslated((double)offset[0]/(mode.w*2), (double)offset[1]/(mode.h*2), 0.0);

	//glTranslated( afx, afy, 0.0 );
	//glScaled(mode.w, mode.h, 1.0);
	
	
	glScaled( scaler_x, scaler_y, 1.0 );
	glMatrixMode( GL_MODELVIEW );
}

bool background::direct() const
{
	return direct_mode;
}

void background::init(screen disp, bgnum bg)
{
	this->disp = disp;
	this->bg = bg;
	control = 0xFFFFFFFF; // force unset at first set_control
	for (int i = 0; i < 16; i++)
		mat[i] = 0.0;
	mat[0] = 1.0;
	mat[5] = 1.0;
	mat[10] = 1.0;
	mat[15] = 1.0;
	affine[0] = 0.0;
	affine[1] = 0.0;
}

void background::set_offset(const bg_offsets &ofs)
{
	offset[0] = ofs.x;
	offset[1] = ofs.y;
}

void background::set_control(unsigned long dctl, unsigned long bgctl)
{
	if ((control == bgctl) && (dcontrol == dctl))
		return;

	// update settings
	control = bgctl;
	dcontrol = dctl;
	calcBases();
	unsigned long bgmode = dcontrol & 0x7;
	int affined_mode = -1;

	direct_mode = false;
	if (control & (1 << 7))
	{
		color_mode   = 8; // 8bpp
		affined_mode = 3;
		if (this->bgmode == BG_AFFINE_EXT)
		{
			if (control & (1 << 2))
			{
				direct_mode = true;
				affined_mode = 4;
			}
		}
	}
	else
	{
		if (this->bgmode == BG_AFFINE_EXT)
			color_mode = 8;
		else color_mode = 4; // 4bpp			
		affined_mode = 2;
	}

	this->bgmode = bgmodes[disp][bgmode][bg].mode;

	
	int moderow = -1;
	switch (this->bgmode)
	{
	case BG_TEXT:
	case BG_TEXT_3D:    moderow = 0; break;
	case BG_AFFINE:     moderow = 1; break;
	case BG_AFFINE_EXT: moderow = affined_mode; break;
	case BG_LARGE:      moderow = 5; break;
	}

	if (moderow < 0)
	{
		// invalid mode
		valid = false;
	} else
	{
		// valid mode selected
		unsigned long ssize = (control >> 14) & 0x3;
		mode = modes[disp][ssize][moderow];
		valid = true;
		tilemode = tilemode_grid[moderow];

		
		if (tilemode == BG_CHAR8)
		{
			tex.resize( 128, 128 );
			chartex.resize(256, 256, 1);
		}
		else 
		{
			tex.resize( mode.w, mode.h );
			chartex.free();
		}
	}

	// check if enabled dcontrol
	unsigned long new_enabled = dcontrol & (1 << (8 + (int)bg));
	ignore_flush = 1; //enabled == new_enabled;
	enabled = new_enabled;
}

// debug helper that randomizes the content of a disabled or invalid BG
void background::bg_off()
{
}

NDSE::memory_block* background::get_vram()
{
	NDSE::memory_block *b = 0;
	switch (disp)
	{
		case DISP_A: b = NDSE::ARM9_GetPage(0x06000000); break;
		case DISP_B: b = NDSE::ARM9_GetPage(0x06200000); break;
		default: b = NDSE::ARM9_GetPage(0x00000000);
	}

	return b;
}

void background::pull_direct(NDSE::memory_block *block) // pulls 1:5:5:5 data
{
	unsigned long update = next_block(block, screen_base >> NDSE::PAGE_BITS) | ignore_flush;
	assert(!(screen_base & NDSE::PAGE_MASK & 0x1FF));
	unsigned char *mem = (unsigned char*)block->mem + (screen_base & NDSE::PAGE_MASK);
	unsigned char *end = (unsigned char*)block->mem + NDSE::PAGE_SIZE; 
	
	unsigned long w = tex.get_w();
	unsigned long w2 = w << 1;
	unsigned long h = tex.get_h();

	// careful only works when w2 <= 512 (PAGE_SIZE)
	assert(w2 <= NDSE::PAGE_SIZE);
	for (unsigned long y = 0; y < h; y++)
	{
		if (update)
		{
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, y, w, 1, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, mem );
		} 
		mem += w2;
		if (mem >= end)
		{
			// load next page
			update = next_block(block) | ignore_flush;
			mem = (unsigned char *)block->mem;
			end = (unsigned char *)block->mem + NDSE::PAGE_SIZE; 
		}
	}
}

void background::pull_bmp256() // pulls 256 color bitmap
{
	assert(!(screen_base & NDSE::PAGE_MASK & 0x1FF));
	NDSE::memory_block *block = get_vram();
	
	unsigned long update = next_block(block, screen_base >> NDSE::PAGE_BITS) | ignore_flush;
	unsigned char *mem = (unsigned char*)block->mem + (screen_base & NDSE::PAGE_MASK);
	unsigned char *end = (unsigned char*)block->mem + NDSE::PAGE_SIZE; 
	
	unsigned long w = tex.get_w();
	unsigned long h = tex.get_h();

	for (unsigned long y = 0; y < h; y++)
	{
		if (update)
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, y, w, 1, GL_ALPHA, GL_UNSIGNED_BYTE, mem );
		mem += w;
		if (mem >= end)
		{
			// load next page
			update = next_block(block) | ignore_flush;
			mem = (unsigned char *)block->mem;
			end = (unsigned char *)block->mem + NDSE::PAGE_SIZE; 
		}
	}
}

// pulls character mode content
// there are at most 1024 and at min 256 8x8 characters
// they will get uploaded to a 256*256 (32x32 chars) texture
// theres a 16 and a 256 color version where the 16 color one
// just uncompresses to 256bit indices on the fly

void background::pull_chars16() // pulls 4bit per char as 8bit red channel
{
	chars_pull16(get_vram(), char_base, ignore_flush);
}

void background::pull_chars256()
{
	chars_pull256(get_vram(), char_base, ignore_flush);
}

void background::pull_charscreen()
{
	unsigned long base = screen_base;
	assert(!(base & NDSE::PAGE_MASK & 0x1FF));
	NDSE::memory_block *block = get_vram();
	unsigned long update = next_block(block, base >> NDSE::PAGE_BITS) | ignore_flush;;
	unsigned short *mem = (unsigned short*)(block->mem + (base & NDSE::PAGE_MASK));
	unsigned short *end = (unsigned short*)(block->mem + NDSE::PAGE_SIZE);

	unsigned long w = mode.w / 8;
	unsigned long h = mode.h / 8;
	static unsigned long scanline[128];

	for (unsigned long y = 0; y < h; y++)
	{
		// each pixel decomposites as
		// xxxx xx xxxxxxxxxx
		// pal  HV   charname

		if (update)
		{
			for (unsigned int x = 0; x < w; x++)
			{
				unsigned long l = *mem++;
				//if ((l & 0x3FF) == 0)
				//	l = '+';
				unsigned long xc  = l & 0x1F;
				unsigned long yc  = (l >> 5) & 0x1F;
				unsigned long pal = l >> 12;
				unsigned long hv  = (l << 20)& 0xC0000000;
				
				scanline[x] = (xc << 3) | (yc << 11) | (pal << 20) | hv;
			}
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, y, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, scanline );
		} else
			mem += w;
	

		if (mem >= end)
		{
			// load next page
			update =next_block(block) | ignore_flush;;
			mem = (unsigned short *)(block->mem);
			end = (unsigned short *)(block->mem + NDSE::PAGE_SIZE); 
		}
	}
}

void background::pull_bg()
{
	if (direct_mode)
	{
		tex.activate();
		return pull_direct( get_vram() );
	}

	if (tilemode == BG_CHAR8)
	{
		chartex.activate();
		//needed for 256_color_bmp.elf etc console demos
		if (color_mode == 4)
			pull_chars16();
		else pull_chars256();
		
		tex.activate();
		pull_charscreen();
		return;
	}

	tex.activate();
	pull_bmp256();
	
}

void background::pull_data()
{
	if (!valid)
		return bg_off();
	if (!enabled)
		return;

	switch (disp_mode)
	{
	case DM_OFF: return bg_off();
	case DM_GRAPHIC: // BG + OBJ
		pull_bg();
	}

	ignore_flush = 0;
}


void background::set_affine( const bg_affine &affine )
{
	// 0 1 2 3
	// 4 5 6 7
	// 8 9 A B
	// C D E F
	
	
	/*
	mat[0] = fixed16_to_double<8>(affine.pa); // x axis transformed -> x'.x
	mat[4] = fixed16_to_double<8>(affine.pb); // x axis transformed -> x'.y
	mat[1] = fixed16_to_double<8>(affine.pc); // y axis transformed -> y'.x
	mat[5] = fixed16_to_double<8>(affine.pd); // y axis transformed -> y'.y
	*/

	// 0 = dx  (pa)
	// 1 = dmx (pb)
	// 4 = dy  (pc)
	// 5 = dmy (pd)

	mat[0] = fixed16_to_double<8>(affine.pa); // x axis transformed -> x'.x
	mat[1] = fixed16_to_double<8>(affine.pb); // x axis transformed -> x'.y
	mat[4] = fixed16_to_double<8>(affine.pc); // y axis transformed -> y'.x
	mat[5] = fixed16_to_double<8>(affine.pd); // y axis transformed -> y'.y
	
	this->affine[0] = fixed28_to_double<8>(affine.x);  // affine.x
	this->affine[1] = fixed28_to_double<8>(affine.y);  // affine.y
	
}
