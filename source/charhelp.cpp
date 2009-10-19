#include "charhelp.h"
#include "shaders.h"
#include "block_helper.h"

void chars_pull16(NDSE::memory_block *base, unsigned long offset, unsigned long force)
{
	assert(!(offset & NDSE::PAGE_MASK & 0x1FF));

	unsigned long update = next_block(base, offset >> NDSE::PAGE_BITS) | force;;
	unsigned char *mem = (unsigned char*)base->mem + (offset & NDSE::PAGE_MASK);
	unsigned char *end = (unsigned char*)base->mem + NDSE::PAGE_SIZE;
	
	unsigned char decompressed[64];
	unsigned char *dptr;
	
	// naive version
	for (unsigned long y = 0; y < 256; y += 8)
	{
		for (unsigned long x = 0; x < 256; x += 8)
		{
			// pull a char
			if (update)
			{
				dptr = decompressed;
				for (unsigned int i = 0; i < 8*4; i++)
				{
					unsigned long v = *mem++;
					//*dptr++ = (unsigned char)(v << 4)   /*| 0x0F*/;
					//*dptr++ = (unsigned char)(v & 0xF0) /*| 0x0F*/;
					*dptr++ = (unsigned char)(v & 0x0F);
					*dptr++ = (unsigned char)(v >> 4);
				}				
				glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, 8, 8, GL_ALPHA, GL_UNSIGNED_BYTE, decompressed );
				
			} else mem += 32;
			if (mem >= end)
			{
				// load next page
				update = next_block(base) | force;
				mem = (unsigned char *)base->mem;
				end = (unsigned char *)base->mem + NDSE::PAGE_SIZE; 
			}
		}
	}
}

void chars_pull256(NDSE::memory_block *base, unsigned long offset, unsigned long force)
{
	assert(!(offset & NDSE::PAGE_MASK & 0x1FF));

	unsigned long update = next_block(base, offset >> NDSE::PAGE_BITS) | force;;
	unsigned char *mem = (unsigned char*)base->mem + (offset & NDSE::PAGE_MASK);
	unsigned char *end = (unsigned char*)base->mem + NDSE::PAGE_SIZE;

	// naive version
	for (unsigned long y = 0; y < 256; y += 8)
	{
		for (unsigned long x = 0; x < 256; x += 8)
		{
			// pull a char
			if (update)
				glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, 8, 8, GL_ALPHA, GL_UNSIGNED_BYTE, mem );
			mem += 8*8;
			if (mem >= end)
			{
				// load next page
				update = next_block(base) | force;;
				mem = (unsigned char *)base->mem;
				end = (unsigned char *)base->mem + NDSE::PAGE_SIZE; 
			}
		}
	}
}
