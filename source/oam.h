#ifndef _OAM_H_
#define _OAM_H_

#include <video/video.h>
#include "shader.h"
#include "ioregs.h"


// might move vram + stride from pull_texture to a set_stride() method
class CSpriteEntry
{
private:
	texture2d tex;
	int x, y;
	bool direct;
	unsigned long mode;
	unsigned long tile;
	unsigned long oldaddr;
	unsigned long hicol;
	mode_setting shape;
public:
	void init();
	void pull(oam_entry *mem, unsigned long &need16, unsigned long &need256);
	void pull_texture(unsigned long obj_vram, unsigned long stride);
	void render();
};

class CSpriteObjects
{
private:
	NDSE::memory_block *boam;
	unsigned long vram;
	CSpriteEntry oam_t[128];
	unsigned long stride;
	texture2d chartex16;
	texture2d chartex256;

	unsigned long need16, need256;

	void pull_charmap();
public:
	void pull();
	void display();
	void init(unsigned long oam_base, unsigned long obj_vram_base);
	void set_control(unsigned long dispcnt);
};

#endif
