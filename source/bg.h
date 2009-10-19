#ifndef _BG_H_
#define _BG_H_

#include <video/video.h>
#include "shader.h"
#include "ioregs.h"

// cleanup vram address init!!

class background: public CAbstractBG
{	
public:
	texture2d tex;
	texture2d chartex;
protected:
	
public:
	tile_mode get_mode();

	void set_transform();
	bool direct() const;

	void init(screen disp, bgnum bg);
	void set_offset(const bg_offsets &ofs);
	void set_control(unsigned long dctl, unsigned long bgctl);

	// debug helper that randomizes the content of a disabled or invalid BG
	void bg_off();
	NDSE::memory_block* get_vram();

	void pull_direct(NDSE::memory_block *block); // pulls 1:5:5:5 data
	void pull_bmp256(); // pulls 256 color bitmap

	// pulls character mode content
	// there are at most 1024 and at min 256 8x8 characters
	// they will get uploaded to a 256*256 (32x32 chars) texture
	// theres a 16 and a 256 color version where the 16 color one
	// just uncompresses to 256bit indices on the fly
	
	void pull_chars16(); // pulls 4bit per char as 8bit red channel
	void pull_chars256();
	void pull_charscreen();
	void pull_bg();
	void pull_data();
	void set_affine( const bg_affine &affine );
};


#endif
