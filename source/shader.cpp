#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <list>
#include <algorithm>
#include <string>

#include "shader.h"

#define SHADER_DEBUG

#ifndef _DEBUG
#undef SHADER_DEBUG
#endif

#ifdef SHADER_DEBUG
#define OutputDebugStringA(s) std::cout << s;
#endif

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
//PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D; // GL 1.3

#ifdef WIN32
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif

#ifdef SHADER_DEBUG
PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
#endif

void error(const char *err)
{
	std::cerr << err << "Please update your graphic drivers/hardware" << std::endl;
	exit(-1);
}

extern void* GetExtension(const char *name);

template <typename T>
void load_ext(T& func, const char *name)
{
	func = (T)GetExtension(name);
	if (!func)
	{
		char buffer[256];
		sprintf( buffer, "Failed to load extension: %s", name );
		error(buffer);
	}
}


static std::list<std::string> extensions;
void check_extension(const char *cstr)
{
	std::string str( cstr );
	if (std::find( extensions.begin(), extensions.end(), str ) == extensions.end() )
	{
		char buffer[256];
		sprintf( buffer, "Missing extension: %s", cstr );
		error(buffer);
	}
}

void gl_shader::init()
{
	// check GL Version
	const char* ver = (char*)glGetString( GL_VERSION );
	const char* ext = (char*)glGetString( GL_EXTENSIONS );
	char buffer[256];

	int major = 0, minor = 0;
	if (sscanf( ver, "%d.%d", &major, &minor ) != 2)
	{
		return error("Malformed GL Version Info.");
	}
	if (major < 2)
	{
		sprintf( buffer, 
			"Your OpenGL Version is too old.\n"
			"Current Version: %s\n"
			"Needed Version: 2.0\n", ver);
		return error(buffer);
	}
	
	const char *p = ext;
	while (*p)
	{
		ext = p;
		while (*p && (*p != ' '))
			p++;
		extensions.push_back( std::string( ext, p - ext ) );
		if (*p)
			p++;
	}

	check_extension("GL_EXT_framebuffer_object");


	// check extensions
	load_ext(glCreateShader, "glCreateShader");
	load_ext(glShaderSource, "glShaderSource");
	load_ext(glCompileShader, "glCompileShader");
	load_ext(glDeleteShader, "glDeleteShader");
	load_ext(glCreateProgram, "glCreateProgram");
	load_ext(glDeleteProgram, "glDeleteProgram");
	load_ext(glAttachShader, "glAttachShader");
	load_ext(glLinkProgram, "glLinkProgram");
	load_ext(glUseProgram, "glUseProgram");
	load_ext(glGetUniformLocation, "glGetUniformLocation");
	load_ext(glUniform1i, "glUniform1i");
	load_ext(glUniform2f, "glUniform2f");
	load_ext(glGenFramebuffersEXT, "glGenFramebuffersEXT");
	load_ext(glDeleteFramebuffersEXT, "glDeleteFramebuffersEXT");
	load_ext(glBindFramebufferEXT, "glBindFramebufferEXT");
	load_ext(glFramebufferTexture2DEXT, "glFramebufferTexture2DEXT");
	load_ext(glCheckFramebufferStatusEXT, "glCheckFramebufferStatusEXT");
	//load_ext(glCompressedTexSubImage2D, "glCompressedTexSubImage2D");

#ifdef WIN32
	load_ext(glActiveTexture, "glActiveTexture");
#endif

#ifdef SHADER_DEBUG
	load_ext(glGetInfoLogARB, "glGetInfoLogARB");
#endif
}

////////////////////////////////////////////////////////////////////////////////

unsigned int adjust_pow2(unsigned int w)
{
	int bits = 0;
	int ones = 0;
	while (w > 1)
	{
		if (w & 1)
			ones++;
		w >>= 1;
		bits++;
	}
	if (ones > 1)
		bits++;
	if (bits < 6)
		bits = 6; // min 64x64 for GL ....

	return 1 << bits;
}


unsigned long texture2d::get_w() const
{ 
	return w; 
}

unsigned long texture2d::get_h() const 
{ 
	return h; 
}

GLuint texture2d::get_name() const
{
	return tex;
}

void texture2d::activate() const
{
	glBindTexture( GL_TEXTURE_2D, tex );
}

void texture2d::free()
{
	if (tex)
		glDeleteTextures( 1, &tex );
	w = 0;
	h = 0;
	tex = 0;
}

void texture2d::resize(unsigned int width, unsigned int height, unsigned int components)
{
	width = adjust_pow2(width);
	height = adjust_pow2(height);

	if ((w == width) && (h == height))
		return;
		
	free();
	w = width;
	h = height;

	// create the texture
	glGenTextures( 1, &tex );

	// allocate mem first time for the texture
	set_filter( FILTER_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLint internalformat = 0;
	GLenum format = 0;
	switch (components)
	{
	case 1: internalformat = GL_ALPHA8; format = GL_ALPHA; break;
	case 3: internalformat = GL_RGB8; format = GL_RGB; break;
	case 4: internalformat = GL_RGBA8; format = GL_RGBA; break;
	}

	//std::cout << "Created Texture: " << w << " x " << h << "\n";
	glTexImage2D( GL_TEXTURE_2D, 0, internalformat, w, h, 0, format, GL_UNSIGNED_BYTE, 0 );
}

void texture2d::set_filter(texture_filter filter)
{
	activate();
	switch (filter)
	{
	case FILTER_NEAREST:
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	case FILTER_LINEAR:
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}

}

texture2d::texture2d()
{
	tex = 0;
}

texture2d::~texture2d()
{
	free();
}



unsigned long texture1d::get_w() const
{ 
	return w;
}
	
void texture1d::activate() const
{
	glBindTexture( GL_TEXTURE_1D, tex );
}

void texture1d::resize(unsigned int width)
{
	width = adjust_pow2(width);

	if (w == width)
		return;
	w = width;

	if (tex)
		glDeleteTextures( 1, &tex );

	// create the texture
	glGenTextures( 1, &tex );
	activate();
		
	// allocate mem first time for the texture
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA8, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
}

texture1d::texture1d()
{
	tex = 0;
}

texture1d::~texture1d()
{
	glDeleteTextures( 1, &tex );
}

////////////////////////////////////////////////////////////////////////////////

void gl_shader::load(shader_type t, const std::string &s)
{
	GLenum glt = 0;
	switch (t)
	{
	case VS: glt = GL_VERTEX_SHADER; break;
	case PS: glt = GL_FRAGMENT_SHADER; break;
	}
	name = glCreateShader(glt);

	const GLchar *codep = (GLchar*)s.c_str();
	GLint len = (GLint)s.length();
	glShaderSource( name, 1, &codep, &len );
	glCompileShader( name );

#ifdef SHADER_DEBUG
	char buffer[1024];
	glGetInfoLogARB( name, sizeof(buffer), &len, buffer );
	if (buffer && *buffer)
	{
		OutputDebugStringA("Shader Compilation Log for:\n");
		OutputDebugStringA( (char*)codep );
		OutputDebugStringA("====================\n");
		OutputDebugStringA( buffer );
		OutputDebugStringA("====================\n");
	}
#endif
}

gl_shader::gl_shader(shader_type t, std::istream &s)
{
	char buffer[1024];
	std::string code;

	while (s.good())
	{
		s.read(buffer, sizeof(buffer));
		code.append( buffer, s.gcount() );
	}
	load(t, code);
}

gl_shader::gl_shader(shader_type t, const std::string &s)
{
	load(t, s);
}

gl_shader::~gl_shader()
{
	glDeleteShader(name);
}

GLuint gl_shader::get_name() const
{
	return name;
}

////////////////////////////////////////////////////////////////////////////////

gl_program::gl_program()
{
	name = glCreateProgram();
}

gl_program::~gl_program()
{
	glDeleteProgram(name);
}

void gl_program::link()
{
	glLinkProgram( name );
}

void gl_program::activate()
{
	glUseProgram( name );
}

void gl_program::finish()
{
	glUseProgram( 0 );
}


void gl_program::attach(const gl_shader* shader)
{
	glAttachShader( name, shader->get_name() );
}

void gl_program::uniform_i(const char *uname, GLint i1)
{
	GLint id = glGetUniformLocation( name, uname );
	glUniform1i( id, i1 );
}

void gl_program::uniform_2f(const char *uname, GLfloat f1, GLfloat f2)
{
	GLint id = glGetUniformLocation( name, uname );
	glUniform2f( id, f1, f2 );
}

void gl_program::glActiveTexture(GLenum tex)
{
	::glActiveTexture( tex );
}

/*
void gl_program::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
	::glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}
*/

////////////////////////////////////////////////////////////////////////////////

gl_fbo::gl_fbo()
{
	glGenFramebuffersEXT( 1, &fbo );
}

gl_fbo::~gl_fbo()
{
	glDeleteFramebuffersEXT( 1, &fbo );
}

void gl_fbo::activate()
{
	check();
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo );
}

void gl_fbo::finish()
{
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}


void gl_fbo::set_tex(texture2d &tex)
{
	activate();
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, 
		GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex.get_name(), 0 );
	finish();
}

void gl_fbo::check()
{
	GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
	assert(status == GL_FRAMEBUFFER_COMPLETE_EXT);
}
