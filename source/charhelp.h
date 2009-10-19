#ifndef _CHARHELP_H_
#define _CHARHELP_H_

#include <video/video.h>

void chars_pull16(NDSE::memory_block *start, unsigned long offset, unsigned long force = 0);
void chars_pull256(NDSE::memory_block *start, unsigned long offset, unsigned long force = 0);


#endif
