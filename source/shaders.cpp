#include "shaders.h"
#include "hq3x.h"
#include <fstream>



shader::shader(std::istream &is) : 
	  vs(gl_shader::VS, std::string(default_vs)), 
	  //vs(gl_shader::VS, std::ifstream("shaders\\default.vs")), 
	  ps(gl_shader::PS, is)
{
	attach( &vs );
	attach( &ps );
	link();
}

  shader::shader(std::string shader) : 
	  vs(gl_shader::VS, std::string(default_vs)), 
	  //vs(gl_shader::VS, std::ifstream("shaders\\default.vs")), 
	  ps(gl_shader::PS, shader)
{
	attach( &vs );
	attach( &ps );
	link();
}




shader *shaders::char256;
shader *shaders::plain256;
//shader *shaders::post_filter;
//shader *shaders::post_rgb;

shader* load_shader(const char* filename)
{
	std::ifstream i(filename);
	return new shader(i);
}

void shaders::init()
{
	plain256 = new shader(std::string(palette_ps));	
	char256 = new shader(std::string(character_ps));
	//char256  = new shader(std::ifstream("shaders\\char256.ps"));
	//post_filter = new shader(std::string(filter_ps));
	//post_filter = new shader_hq3x();
	//post_filter = load_shader("shaders\\rgb.ps");


	//post_filter = new shader(std::ifstream("shaders\\filter.ps"));
	//post_rgb    = load_shader("shaders\\rgb.ps");
	//post_hq3x = new shader_hq3x();
}


////////////////////////////////////////////////////////////////////////////////
// Embedded shader sourcecode starts here

const char *default_vs = 
	"void main()"
	"{"
		"gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
		"gl_Position = ftransform();"
	"}";

const char *palette_ps =
	"uniform sampler1D palette;"
	"uniform sampler2D bg;"
	"void main()"
	"{"
		"float idx = texture2D( bg, gl_TexCoord[0].st ).a;"
		"gl_FragColor = texture1D( palette, idx*255.0/256.0 + 0.5/256.0 );" // or just * 0.999 fix
		//"gl_FragColor = vec4(gl_TexCoord[0].xyz, 0.0);"
		// isnt there any nicer way?!
		//"gl_FragColor = texture1D( palette, idx + 1.0/256.0 );"
	"}";

const char *character_ps =
	"uniform sampler1D palette;"
	"uniform sampler2D bg;"
	"uniform sampler2D charmap;"
	"void main()"
	"{"
		"vec2 p = gl_TexCoord[0].st;"
		"vec2 subpos  = mod(p,1.0/128.0)*4.0;"
		"vec2 fsubpos = vec2(4.0/128.0) - subpos;"
		"vec4 chr = texture2D(bg, p);"
		//"chr.rg *= 255.0/256.0;"
		"chr.rg = floor(chr.rg*256.0)/256.0;"
		
		"float vflip = floor(chr.a + 0.6);"
		"float hflip = ceil(chr.a - 0.5 * vflip - 0.2 );"
		"subpos = mix(subpos, fsubpos,  vec2(hflip, vflip));"
		
		"subpos += chr.rg;"
		"float idx = texture2D( charmap, subpos ).a + chr.b;"
		"gl_FragColor = texture1D( palette, idx*255.0/256.0 + 0.5/256.0 );"
	"}"
	;

const char *filter_ps =
	"uniform sampler2D frame;"
	"uniform vec2 size;"
	"vec2 inv_size = vec2(1.0/size.x, 1.0/size.y);"
	"float threshold = 0.001;"
	"void main()"
	"{"
		"vec2 sub_pos = step( mod( gl_TexCoord[0].st, inv_size ) * 2.0 , inv_size);"
		"vec2 off = sub_pos * 2.0 - 1.0;"
		"off *= inv_size;"

		"vec4 p = texture2D( frame, gl_TexCoord[0].st );"
		"vec4 a = texture2D( frame, gl_TexCoord[0].st - vec2( off.x, 0.0 ) );"
		"vec4 b = texture2D( frame, gl_TexCoord[0].st - vec2( 0.0, off.y ) );"
	
		"gl_FragColor = mix( p, 0.5*(p+a), step( abs(a - b), vec4(threshold) ) );"
	"}";


const char *filter_hq3x =
"uniform sampler2D frame;"
"uniform sampler2D hq3x_lut;"
"uniform vec2 size;"
"vec2 inv_size = vec2(1.0/size.x, 1.0/size.y);"

"vec3 hqx_threshold = vec3("
  "0.0023529411764705882352941176470588,"
  "0.027450980392156862745098039215686,"
  "0.1882352941176471"
");"

"mat3 rgb_to_yuv = mat3("
   "0.299,    0.587,    0.114,"
   "-0.14713, -0.28886,  0.436,"
   "0.615,   -0.51499, -0.10001"
");"

"float compare2hqx( vec4 a, vec4 b )"
"{"
   "vec3 y1 = rgb_to_yuv * a.rgb;"
   "vec3 y2 = rgb_to_yuv * b.rgb;"
   "vec3 ydiff = step(hqx_threshold, abs(y1 - y2));"
   "return clamp( dot(ydiff, ydiff), 0.0, 1.0 );"
"}"


"vec4 hq3x(void)"
"{"
   // todo once this works:
   // wrap stuff to matrices/vectors so summing products gets
   // less operations
   "vec2 t = gl_TexCoord[0].st*1.0;" // zoom for debugging
   "vec2 sub_pos = floor(mod( t, inv_size ) * size * 3.0);"
   "float didx = (sub_pos.x + sub_pos.y * 3.0);" // [0..8]
   
   //return vec4(didx/8.0);
   

   "vec4 a = texture2D( frame, t - inv_size );"                       // topleft
   "vec4 b = texture2D( frame, t - vec2( 0.0, inv_size.y) );"         // top
   "vec4 c = texture2D( frame, t + vec2( inv_size.x, -inv_size.y) );" // topright
   "vec4 d = texture2D( frame, t - vec2( inv_size.x, 0.0) );"         // left
   "vec4 e = texture2D( frame, t );"                                  // center
   "vec4 f = texture2D( frame, t + vec2( inv_size.x, 0.0) );"         // right
   "vec4 g = texture2D( frame, t - vec2( inv_size.x, -inv_size.y) );" // bottomleft
   "vec4 h = texture2D( frame, t + vec2( 0.0, inv_size.y) );"         // bottom
   "vec4 i = texture2D( frame, t + inv_size );"                       // bottomright
   
   
   /*
   float idx = 
   compare2( a, e, threshold ) * 0.5 +
   compare2( b, e, threshold ) * 0.25 +
   compare2( c, e, threshold ) * 0.125 +
   compare2( d, e, threshold ) * 0.0625 +
   compare2( f, e, threshold ) * 0.03125 +
   compare2( g, e, threshold ) * 0.015625 +
   compare2( h, e, threshold ) * 0.0078125 +
   compare2( i, e, threshold ) * 0.00390625 +
   */
   
   // more secure to start testing with
   
   // select the LUT slice
   "float idx ="
   "compare2hqx( a, e ) * 1.0 +"
   "compare2hqx( b, e ) * 2.0 +"
   "compare2hqx( c, e ) * 4.0 +"
   "compare2hqx( d, e ) * 8.0 +"
   "compare2hqx( f, e ) * 16.0 +"
   "compare2hqx( g, e ) * 32.0 +"
   "compare2hqx( h, e ) * 64.0 +"
   "compare2hqx( i, e ) * 128.0;"
   
   // calculate the LUT subindex
   "float subidx ="
   "compare2hqx( b, f ) * 1.0 +"
   "compare2hqx( f, h ) * 2.0 +"
   "compare2hqx( h, d ) * 4.0 +"
   "compare2hqx( d, b ) * 8.0;" // [0..15]
   "subidx *= 9.0;"
   "subidx += didx;"
   "subidx *= 3.0;"
   
   "vec4 r1 = texture2D( hq3x_lut, vec2((subidx      )/511.0, idx/255.0) );"
   "vec4 r2 = texture2D( hq3x_lut, vec2((subidx + 1.0)/511.0, idx/255.0) );"
   "vec4 r3 = texture2D( hq3x_lut, vec2((subidx + 2.0)/511.0, idx/255.0) );"
   
   //return r1;
   
   //return e;
   
   "return "
     "a * r1.z + b * r1.y + c * r1.x +"
     "d * r2.z + e * r2.y + f * r2.x +"
     "g * r3.z + h * r3.y + i * r3.x;"
   
   
   
   //return e;
"}"

"void main(void)"
"{"
	"gl_FragColor = hq3x();"
"}"
;

