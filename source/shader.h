#ifndef _SHADER_H_
#define _SHADER_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#include <iostream>


enum texture_filter { FILTER_LINEAR, FILTER_NEAREST };

class texture2d
{
	unsigned long w, h;
	GLuint tex;
public:
	unsigned long get_w() const;
	unsigned long get_h() const;
	GLuint get_name() const;
	void activate() const;
	void free();
	void resize(unsigned int width, unsigned int height, unsigned int components = 4);
	void set_filter(texture_filter filter);
	texture2d();
	~texture2d();
};


class texture1d
{
	unsigned long w;
	GLuint tex;
public:
	unsigned long get_w() const;
	void activate() const;
	void resize(unsigned int width);
	texture1d();
	~texture1d();
};


class gl_shader
{
public:
	enum shader_type { PS, VS };
private:
	GLuint name;
	void load(shader_type t, const std::string &s);
public:
	static void init();

	GLuint get_name() const;
	gl_shader(shader_type t, std::istream &s);
	gl_shader(shader_type t, const std::string &s);
	~gl_shader();
};

class gl_program
{
private:
	GLuint name;
public:
	gl_program();
	~gl_program();

	void link();
	virtual void activate();
	static void finish();

	void uniform_i(const char *uname, GLint i1);
	void uniform_2f(const char *uname, GLfloat f1, GLfloat f2);

	void attach(const gl_shader* shader);
	static void glActiveTexture(GLenum tex);
	//static void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, 
	//	GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
};

class gl_fbo
{
private:
	GLuint fbo;
public:
	gl_fbo();
	~gl_fbo();
	void activate();
	static void finish();

	void set_tex(texture2d &tex);
	void check();
};

class render_to_texture
{
public:
	texture2d tex;
	gl_fbo fbo;
	render_to_texture(int w, int h)
	{
		tex.resize(w, h);
		fbo.set_tex( tex );
	}
};


#endif
