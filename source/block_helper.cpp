#include <cstring>
#include <assert.h>
#include "block_helper.h"

#ifdef __GNUC__
#include "intrin_x86.h"
#else
#include <intrin.h>
#pragma intrinsic (_InterlockedAnd)
#endif

static int flush_ctr = 0;

static NDSE::memory_block *flush_buffer[FLUSHBUFFER_SIZE];
static unsigned long dirty_table[FLUSHBUFFER_SIZE];

unsigned long is_dirty(NDSE::memory_block *page)
{
        for (int i = 0; i < flush_ctr; i++)
                if (flush_buffer[i] == page)
                        return dirty_table[i] | DEBUG_DIRTY;

        unsigned long dirty =
                _InterlockedAnd( (long*)&page->flags,
                                 ~NDSE::memory_block::PAGE_DIRTY_VRAM )
                        & NDSE::memory_block::PAGE_DIRTY_VRAM;

        assert(flush_ctr < FLUSHBUFFER_SIZE);
        dirty_table[flush_ctr] = dirty;
        flush_buffer[flush_ctr] = page;
        flush_ctr++;
        return dirty | DEBUG_DIRTY;
}


void flush_now()
{
        flush_ctr = 0;
}

