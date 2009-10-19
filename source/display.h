#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "bg.h"
#include "oam.h"

class display
{
private:
	NDSE::memory_block *bpal;
	char *ppal;
	unsigned long dispcnt;
	NDSE::memory_block *bregs;
	graphic_registers *pregs;
	CSpriteObjects oam;
	
	static unsigned long rgb16_to_rgb32(unsigned long rgb16);
	void prepare_shader(background &bg);
protected:
	void pull_control();
	void pull_pal();
	void init(unsigned long ioregs_addr, unsigned long pal_base,
		unsigned long oam_bas, unsigned long vram_base);
public:
	background bg[4];
	texture1d bg_pal;

	display(screen s);
	void pull_all();
	void set_regs(const graphic_registers *regs);
	void do_display(screen scr, int w, int h);
};


#endif
