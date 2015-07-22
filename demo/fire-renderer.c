#include "fire-renderer.h"
#include "program.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fire-renderer.h"
#include "fire-colormap.h"

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#else 
	#include <GL/glew.h>
#endif

/******************************************************************************/

#define TO_STRING(x) #x
static const char* g_vertex_shader = 
	"#version 150\n"
TO_STRING(
	in vec2 vin_position;
	out vec2 vout_tex_coord;
	
	void main()
	{
		vout_tex_coord.x = 0.5 * (vin_position.x + 1.0);
		vout_tex_coord.y = 0.5 * (vin_position.y + 1.0);
		gl_Position = vec4(vin_position, 0.0, 1.0);
	}
);

static const char* g_fragment_shader =
	"#version 150\n"
TO_STRING(
//	uniform sampler2D u_temperature;
	uniform sampler2D u_smoke_density;
	uniform sampler1D u_alpha_spec;
	uniform sampler1D u_firemap;
	uniform sampler2D u_ignition_coord;

//	uniform float u_temp_min;
//	uniform float u_temp_max;
	
	in vec2 vout_tex_coord;
	
	out vec4 fout_frag_color;
	
	void main()
	{
		float ic = texture(u_ignition_coord, vout_tex_coord).r;
		ic = clamp(ic, 0.0, 1.0);
		float ic_idx = 1.0 - ic;
		//float t = 1.0 - (texture(u_temperature, vout_tex_coord).r - 
		//	u_temp_min) / (u_temp_max - u_temp_min);
		//t = clamp(t, 0.01, 0.99);
		vec3 c = texture(u_firemap, ic_idx).rgb;
		float sd = clamp(texture(u_smoke_density,
			vout_tex_coord).r, 0.0, 0.99);
		float a = texture(u_alpha_spec, sd).r;
		fout_frag_color = vec4(c, a);
	}
);

/******************************************************************************/

static float g_quad_vertices[] = {
	-1.0, -1.0,
	1.0, -1.0,
	1.0, 1.0,

	-1.0, -1.0,
	1.0, 1.0,
	-1.0, 1.0
};

static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_buffer = 0;

/* textures used by the renderer */
static GLuint g_temperature_tex = 0;
static GLuint g_smoke_dens_tex = 0;
static GLuint g_ignitition_coord_tex = 0;
static GLuint g_firemap_tex = 0;
static const unsigned int g_spec_sample_count = 100;
static GLuint g_alpha_spec = 0;

static float g_temperature_min = 0.0;
static float g_temperature_max = 1000.0;

/* # of texture samples in both direction */
static GLsizei g_cell_count_i = 0;
static GLsizei g_cell_count_j = 0;

/******************************************************************************/

static void init_program()
{
	g_program = glCreateProgram();
	glueProgramAttachShaderWithSource(g_program, GL_VERTEX_SHADER,
		g_vertex_shader);
	glueProgramAttachShaderWithSource(g_program, GL_FRAGMENT_SHADER,
		g_fragment_shader);
	glBindAttribLocation(g_program, 0, "vin_position");
	glBindFragDataLocation(g_program, 0, "fout_frag_color");
	glueProgramLink(g_program);
	assert(GL_NO_ERROR == glGetError());
}

static void init_geometry()
{
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	glGenBuffers(1, &g_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, g_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertices), g_quad_vertices,
		GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	assert(GL_NO_ERROR == glGetError());
}

static void set_alpha_spec(float (*spec)(float x))
{
	float* s = malloc(g_spec_sample_count * sizeof(*s));
	unsigned int i = 0;
	float dx = 1.0 / (g_spec_sample_count - 1);

	for (i = 0; i < g_spec_sample_count; i++) {
		s[i] = spec(dx * i);
	}

	glBindTexture(GL_TEXTURE_1D, g_alpha_spec);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, g_spec_sample_count, GL_RED,
		GL_FLOAT, s);
	free(s);
}

static float eval_linear_spec(float x)
{
	return x;
}

static void init_textures(GLsizei width, GLsizei height)
{
	glGenTextures(1, &g_temperature_tex);
	glBindTexture(GL_TEXTURE_2D, g_temperature_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED,
		GL_FLOAT, NULL);

	glGenTextures(1, &g_smoke_dens_tex);
	glBindTexture(GL_TEXTURE_2D, g_smoke_dens_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED,
		GL_FLOAT, NULL);

	glGenTextures(1, &g_ignitition_coord_tex);
	glBindTexture(GL_TEXTURE_2D, g_ignitition_coord_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
		GL_FLOAT, NULL);

	glGenTextures(1, &g_alpha_spec);
	glBindTexture(GL_TEXTURE_1D, g_alpha_spec);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, g_spec_sample_count, 0, GL_RED,
		GL_FLOAT, NULL);
		
	set_alpha_spec(eval_linear_spec);
	
	glGenTextures(1, &g_firemap_tex);
	glBindTexture(GL_TEXTURE_1D, g_firemap_tex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB,
		(sizeof(g_fire_colormap)) / (3 * sizeof(unsigned char)), 0,
		GL_RGB, GL_UNSIGNED_BYTE, g_fire_colormap);
	assert(GL_NO_ERROR == glGetError());
}

/******************************************************************************/

void fire_renderer_initialize(int cell_count_i, int cell_count_j)
{
	init_program();
	init_geometry();
	init_textures(cell_count_i, cell_count_j);
	g_cell_count_i = cell_count_i;
	g_cell_count_j = cell_count_j;
}

void fire_renderer_set_alpha_spec(float (*spec)(float x))
{
	set_alpha_spec(spec);
}

void fire_renderer_set_temperature_bounds(float min, float max)
{
	g_temperature_min = min;
	g_temperature_max = max;
}

void fire_renderer_finalize()
{
	glDeleteTextures(1, &g_smoke_dens_tex);
	glDeleteTextures(1, &g_temperature_tex);
	glDeleteTextures(1, &g_firemap_tex);
	glDeleteTextures(1, &g_alpha_spec);
	glDeleteProgram(g_program);
	glDeleteBuffers(1, &g_buffer);
	glDeleteVertexArrays(1, &g_vao);
}

void fire_renderer_render(const float* const smoke_densities,
	const float* const temperatures,
	const float* const iginition_coordinates)
{
	/* update textures */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_smoke_dens_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_cell_count_i, g_cell_count_j,
		GL_RED, GL_FLOAT, smoke_densities);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(gl_texture_2d, g_temperature_tex);
	//glTexSubImage2D(gl_texture_2d, 0, 0, 0, g_cell_count_i, g_cell_count_j,
	//	gl_red, gl_float, temperatures);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, g_alpha_spec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_1D, g_firemap_tex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_ignitition_coord_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_cell_count_i, g_cell_count_j,
		GL_RED, GL_FLOAT, iginition_coordinates);

	/* set textures in the program */
	glUseProgram(g_program);
	glueProgramUniform1i(g_program, "u_smoke_density", 0);
//	glueProgramUniform1i(g_program, "u_temperature", 1);
	glueProgramUniform1i(g_program, "u_alpha_spec", 2);
	glueProgramUniform1i(g_program, "u_firemap", 3);		
	glueProgramUniform1i(g_program, "u_ignition_coord", 4);

//	glueProgramUniform1f(g_program, "u_temp_min", g_temperature_min);
//	glueProgramUniform1f(g_program, "u_temp_max", g_temperature_max);
	
	/* render */
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBindVertexArray(g_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	assert(GL_NO_ERROR == glGetError());
}












