#ifndef _VRAM_REACTOR_H_
#define _VRAM_REACTOR_H_

#undef STDCALL
#undef IMPORT

#ifdef WIN32
#define STDCALL __stdcall 
#else
#define STDCALL
#endif

#ifndef EXPORT
	#ifdef WIN32
	#define IMPORT __declspec(dllimport)
	#else
	#define IMPORT
	#endif
#else
	#ifdef WIN32
	#define IMPORT EXPORT
	#else
	#define IMPORT __attribute__((visibility("default")))
	#endif
#endif


extern "C"{
typedef void* (STDCALL *GetExtensionCB)(const char *name);
IMPORT void STDCALL VideoInit2(GetExtensionCB cb);
IMPORT void STDCALL Redisplay(int w, int h);
}

#endif
