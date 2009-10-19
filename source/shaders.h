#ifndef _SHADERS_H_
#define _SHADERS_H_

extern const char *default_vs; 
extern const char *palette_ps;
extern const char *character_ps;
extern const char *filter_ps;
extern const char *filter_hq3x;

#include "shader.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

template <typename T>
class shared_pimpl
{
public:
	typedef boost::shared_ptr<T> pointer;
private:
	pointer p;
public:
	pointer get()
	{
		if (!p)
			p = pointer(new T());
		return p;
	}
};

class shader: public gl_program
{
private:
	gl_shader vs;
	gl_shader ps;
public:
	shader(std::istream &is);
	shader(std::string shader);
};


struct shaders
{
	static shader *char256;
	static shader *plain256;
	static shader *post_filter;
	// static shader *post_rgb;

	static void init();
};

//typedef shared_singleton<shader_> shader;

#endif
