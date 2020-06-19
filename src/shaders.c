
#include "shaders.h"
#include "utils.h"
#include "srcinsert.h"
#include <stdio.h>
#include <GL/glew.h>

/* Wrapper around glCompileShader that prints out the error message to stderr
 * if the shader compilation fails. The dbg_info parameter can be NULL, but
 * should actually provide information that describe the shader type and shader
 * source given to glShaderSource when creating the shader (like a file path)
 * to be added in the error message if any.
 * Return -1 if anything goes wrong, return 0 if everything goes well. */
static int compile_shader(GLuint shader_id, const char* dbg_info)
{
	glCompileShader(shader_id);
	GLint status = 0;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
		GLchar* message = xmalloc(length * sizeof(GLchar));
		glGetShaderInfoLog(shader_id, length, &length, message);
		fprintf(stderr,
			"OpenGL shader compilation error (%s): \x1b[31m\"%s\"\x1b[39m\n",
			(dbg_info != NULL) ? dbg_info : "\0", (const char*)message);
		return -1;
	}
	return 0;
}

/* Create an OpenGL shader object with the given glsl source code, then compile
 * it and return its id. */
static GLuint shader_src_compile(const char* shader_src, GLuint shader_type)
{
	GLuint shader_id = glCreateShader(shader_type);
	glShaderSource(shader_id, 1,
		(const GLchar* const*)&shader_src, NULL);
	compile_shader(shader_id, NULL);
	return shader_id;
}

/* Create an OpenGL shader program object and attach the given shaders to it,
 * then link it and return its id.
 * The src_geom parameter can be NULL as a geometry shader is optional in the
 * OpenGL rendering pipeline. */
static GLuint shprog_build(
	const char* src_vert, const char* src_geom, const char* src_frag)
{
	GLuint shprog_id = glCreateProgram();
	GLuint shader_vert_id = shader_src_compile(src_vert, GL_VERTEX_SHADER);
	glAttachShader(shprog_id, shader_vert_id);
	GLuint shader_geom_id;
	if (src_geom != NULL)
	{
		shader_geom_id = shader_src_compile(src_geom, GL_GEOMETRY_SHADER);
		glAttachShader(shprog_id, shader_geom_id);
	}
	GLuint shader_frag_id = shader_src_compile(src_frag, GL_FRAGMENT_SHADER);
	glAttachShader(shprog_id, shader_frag_id);
	glLinkProgram(shprog_id);
	glDetachShader(shprog_id, shader_vert_id);
	glDeleteShader(shader_vert_id);
	if (src_geom != NULL)
	{
		glDetachShader(shprog_id, shader_geom_id);
		glDeleteShader(shader_geom_id);
	}
	glDetachShader(shprog_id, shader_frag_id);
	glDeleteShader(shader_frag_id);
	return shprog_id;
}

/* Create an OpenGL shader program object and attach the given compute shader
 * to it, then link it and return its id. */
static GLuint shprog_compute_build(const char* src_comp)
{
	GLuint shprog_id = glCreateProgram();
	GLuint shader_comp_id = shader_src_compile(src_comp, GL_COMPUTE_SHADER);
	glAttachShader(shprog_id, shader_comp_id);
	glLinkProgram(shprog_id);
	glDetachShader(shprog_id, shader_comp_id);
	glDeleteShader(shader_comp_id);
	return shprog_id;
}

GLuint g_shprog_draw_fade = 0;
GLuint g_shprog_draw_particles = 0;
GLuint g_shprog_comp_iteruniv = 0;

void shprog_build_all(void)
{
	shprog_build_fade();
	shprog_build_particles();
	shprog_build_iteruniv();
}

void shprog_build_fade(void)
{
	g_shprog_draw_fade = shprog_build(
		g_srcinserted_fade_vert,
		g_srcinserted_fade_geom,
		g_srcinserted_fade_frag);
}

void shprog_build_particles(void)
{
	g_shprog_draw_particles = shprog_build(
		g_srcinserted_particles_vert,
		g_srcinserted_particles_geom,
		g_srcinserted_particles_frag);
}

void shprog_build_iteruniv(void)
{
	g_shprog_comp_iteruniv = shprog_compute_build(
		g_srcinserted_iteruniv_comp);
}
