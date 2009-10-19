#include <cstring>
#include "oam.h"
#include "video/video.h"
#include "ioregs.h"
#include "block_helper.h"
#include "charhelp.h"
#include "shaders.h"

#include <fstream>

//#define OAMS

void CSpriteEntry::init()
{
	tex.resize(64, 64);
	x = 0;
	y = 0;
	shape = oam_modes[0][0];
	mode = 0;
	tile = 0;
}



void CSpriteEntry::pull_texture(unsigned long obj_vram, unsigned long stride)
{
	switch (mode)
	{
	case OM_BITMAP:
		tex.activate();

		// pull direct VRAM
		// gbatek:
		// In 1D mapping mode, the Tile Number is simply multiplied by the boundary value.
		// 1D_BitmapVramAddress = TileNumber(0..3FFh) * BoundaryValue(128..256)
		// 2D_BitmapVramAddress = (TileNo AND MaskX)*10h + (TileNo AND NOT MaskX)*80
		// In 2D mode, the Tile Number is split into X and Y indices, the X index is located in the LSBs (ie. MaskX=0Fh, or MaskX=1Fh, depending on DISPCNT.5).
		if (stride)
		{
			// 2d mode
		} else
		{
			// 1d mode

			unsigned long addr = obj_vram + tile * /*shape.pixels * 2*/ 128*2;
			NDSE::memory_block *block = NDSE::ARM9_GetPage(addr);
			unsigned char *mem = (unsigned char*)block->mem + (addr & NDSE::PAGE_MASK);
			unsigned char *end = (unsigned char*)block->mem + NDSE::PAGE_SIZE; 
			unsigned long update = is_dirty(block) | (addr != oldaddr);
			
			
			for (int y = 0; y < shape.h; y += 64/shape.w)
			{
				//for (int x = 0; x < shape.w; x += 64)
				{
					if (update)
					{
						// this is 2D
						//glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, 8, 8, 
						//	GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, mem );

						glTexSubImage2D( GL_TEXTURE_2D, 0, 0, y, shape.w, 64/shape.w, 
							GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, mem );
					}
					mem += 8*8*2;
					if (mem >= end)
					{
						// next page
						update = next_block(block) | (addr != oldaddr);
						mem = (unsigned char *)block->mem;
						end = (unsigned char *)block->mem + NDSE::PAGE_SIZE; 
					}
				}
			}
			oldaddr = addr;
		}
		break;
	default:
		;
	}
}

void CSpriteEntry::pull(oam_entry *mem, unsigned long &need16, unsigned long &need256)
{
	// update the sprite here
	y = mem->attr0 & 0xFF; // y
	x = mem->attr1 & 0x1FF; // x
	mode = (mem->attr0 >> 10) & 3;
	shape = oam_modes[mem->attr0 >> 14][mem->attr1 >> 14];
	tile = mem->attr2 & 0x3FF;
	hicol = (mem->attr0 >> 13) & 1;
	if (mode != OM_BITMAP)
	{
		if (hicol)
			need256 |= 1;
		else need16 |= 1;
	}
}

void CSpriteEntry::render()
{
	tex.activate();
	glPushMatrix();
	glTranslatef((float)x, (float)(192 - y - shape.h), 0.0f);		
	glScalef((float)shape.w, (float)shape.h, 1.0f);
	glBegin(GL_QUADS);
	float tw = 0.015625f * shape.w;
	float th = 0.015625f * shape.h;
	glTexCoord2f(0.0f, th);   glVertex2f( 0.0f, 0.0f);
	glTexCoord2f(tw,   th);   glVertex2f( 1.0f, 0.0f);
	glTexCoord2f(tw,   0.0f); glVertex2f( 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex2f( 0.0f, 1.0f);
	glEnd();
	glPopMatrix();
}

////////////////////////////////////////////////////////////////////////////////


void CSpriteObjects::pull_charmap()
{
	NDSE::memory_block *base = NDSE::ARM9_GetPage(vram);
	if (need16)
	{
		chartex16.activate();
		chars_pull16(base, 0, 1);
	}
	if (need256)
	{
		chartex256.activate();
		chars_pull256(base, 0, 1);
	}
}

void CSpriteObjects::set_control(unsigned long dispcnt)
{
	// 1d basically is same as 2d but with 0 stride
	static unsigned long stride_tab[4] = {128, 256, 0, 0}; // 2d 2d 1d undef
	stride = stride_tab[(dispcnt >> 5) & 3];
}

void CSpriteObjects::init(unsigned long oam_base, unsigned long obj_vram_base)
{
	boam = NDSE::ARM9_GetPage(oam_base);
	vram = obj_vram_base;
	// 64x64 = minimum texture size for gl
	// and maximum for ds OAM unfortunally requires 2mb gpu mem
	for (int i = 0; i < 128; i++)
		oam_t[i].init();

	chartex16.resize(256, 256);
	chartex256.resize(256, 256);
}

void CSpriteObjects::pull()
{
	// OAM is 1kb minimum page size 512bytes
	// so take care of swapping here

	need16  = 0;
	need256 = 0;

	// pull OAM data
	NDSE::memory_block *b = boam;
	oam_entry* mem = (oam_entry*)b->mem;
	oam_entry* end = (oam_entry*)((char*)b->mem + NDSE::PAGE_SIZE); 
	unsigned long dirty = is_dirty(b);
	for (int i = 0; i < 128; i++)
	{
		if (dirty)
			oam_t[i].pull(mem, need16, need256);
		mem++;
		if (mem >= end)
		{
			dirty = next_block(b) /*| ignore_flush*/;
			mem = (oam_entry*)b->mem;
			end = (oam_entry*)((char*)b->mem + NDSE::PAGE_SIZE); 
		}
	}

	
	for (int i = 0; i < 128; i++)
		oam_t[i].pull_texture(vram, stride);

	pull_charmap();
}

void CSpriteObjects::display()
{
#ifdef OAMS // debug
	// render them
	glPushMatrix();
	glTranslatef(-1, -1, 0);
	glScaled(2.0/256.0, 2.0/192.0, 1.0);

	//glPolygonMode(GL_FRONT, GL_LINE);
	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < 128; i++)
		oam_t[i].render();
	glDisable(GL_TEXTURE_2D);
	//glPolygonMode(GL_FRONT, GL_FILL);
	glPopMatrix();

	{
		static shader *debug = new shader(std::ifstream("C:\\Sources\\NDSE\\null.ps"));

		debug->activate();
		debug->glActiveTexture( GL_TEXTURE0 );
		debug->uniform_i("charmap", 0);
		chartex256.activate();
	
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,-1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,-1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
		glEnd();
		debug->finish();
	}
#endif
}
