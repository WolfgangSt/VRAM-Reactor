#ifndef _IOREGS_H_
#define _IOREGS_H_


struct bg_offsets
{
	signed short x;
	signed short y;
};

struct bg_affine
{
	unsigned short pa;
	unsigned short pb;
	unsigned short pc;
	unsigned short pd;
	unsigned long x;
	unsigned long y;
};

// start at 0x04000000 for display A
// start at 0x04001000 for display B
struct graphic_registers
{
	unsigned long dispcnt;        // 00
	unsigned short dispstat;      // 04
	unsigned short vcount;        // 06  only A
	unsigned short bgcnt[4];      // 08
	bg_offsets bgofs[4];          // 10
	bg_affine bg2affine;          // 20
	bg_affine bg3affine;          // 30
	
	unsigned short win0h;         // 40
	unsigned short win1h;         // 42
	unsigned short win0v;         // 44
	unsigned short win1v;         // 46
	unsigned short winin;         // 48
	unsigned short winout;        // 4A
	unsigned short mosaic;        // 4C
	unsigned short unk0;          // 4E
	
	unsigned short bldcnt;        // 50
	unsigned short bldalpha;      // 52
	unsigned short bldy;          // 54
	unsigned short unk1[5];       // 56
	unsigned short disp3dcnt;     // 60 only A
	unsigned short unk2;          // 62
	unsigned long dispcapcnt;     // 64 only A
	unsigned long disp_mmen_fifo; // 68 only A
	unsigned short master_bright; // 6C
	unsigned short unk3;          // 6E pad to 0x70 size
};


struct oam_entry
{
	unsigned short attr0;
	unsigned short attr1;
	unsigned short attr2;
	unsigned short affine;
};

// start at 0x07000000 for display A
// start at 0x07000400 for display B
typedef oam_entry oam[128];


#endif
