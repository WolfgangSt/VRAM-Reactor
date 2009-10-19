#ifndef BLOCK_HELPER_H
#define BLOCK_HELPER_H

namespace NDSE{
#include <NDSE_SDK.h>
};

enum { FLUSHBUFFER_SIZE = 4096 }; // buffer size
enum { DEBUG_DIRTY = 0 };         // 1 = always return dirty

unsigned long is_dirty(NDSE::memory_block *page);
void flush_now();

extern const unsigned long PAGE_SIZE;
__inline unsigned long next_block(NDSE::memory_block* &block)
{
	block = (NDSE::memory_block*)((char*)(block+1) + NDSE::PAGE_SIZE);
	return is_dirty(block);
}

__inline unsigned long next_block(NDSE::memory_block* &block, int i)
{
	block = (NDSE::memory_block*)((char*)(block+i) + i*NDSE::PAGE_SIZE);
	return is_dirty(block);
}

#endif // BLOCK_HELPER_H
