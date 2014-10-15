#include "quantity-renderer.h"
#include "program.h"
#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#endif

GLuint g_program = 0;
GLuint g_vao = 0;
GLuint g_quad_buffer = 0;
GLuint g_texture = 0;
int g_cell_count_i = 0;
int g_cell_count_j = 0;
float g_quantity_min = 0.0;
float g_quantity_max = 1.0;
float g_alpha = 0.6;

static float g_quad[] = {
		-1.0, -1.0,
 		1.0, 1.0,
		-1.0, 1.0,

		-1.0, -1.0,
 		1.0, -1.0,
		1.0, 1.0
	};

/* shader definitions */

#define TO_STRING(x) #x
static const char* g_vertex_shader =
	"#version 150\n"
TO_STRING(
	in vec2 in_position;
	out vec2 vary_tex_coord;
	
	void main()
	{
		vary_tex_coord.x = (in_position.x + 1.0) / 2.0;
		vary_tex_coord.y = (in_position.y + 1.0) / 2.0;
		gl_Position = vec4(in_position, 0.0, 1.0);
	}

);

static const char* g_fragment_shader =
	"#version 150\n"
TO_STRING(
	uniform sampler2D u_quantity_tex;
	uniform float u_alpha;
	uniform float u_quantity_min; /* lowest value a quantity should take */
	uniform float u_quantity_max; /* highest value a quantity should take */
	in vec2 vary_tex_coord;
	out vec4 out_frag_color;
	
	vec3 to_color(float v, float vmin, float vmax)
	{
		vec3 c = vec3(1.0);
		float dv;
	
		if (v < vmin)
			v = vmin;
		if (v > vmax)
			v = vmax;
		dv = vmax - vmin;
	
		if (v < (vmin + 0.25 * dv)) {
			c.r = 0;
			c.g = 4 * (v - vmin) / dv;
		} else if (v < (vmin + 0.5 * dv)) {
			c.r = 0;
			c.b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
		} else if (v < (vmin + 0.75 * dv)) {
			c.r = 4 * (v - vmin - 0.5 * dv) / dv;
			c.b = 0;
		} else {
			c.g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
			c.b = 0;
		}
	
		return c ;
	}
	
	void main()
	{
		float phi = texture(u_quantity_tex, vary_tex_coord).r;
		phi = (phi - u_quantity_min) / (u_quantity_max - u_quantity_min);
		out_frag_color = vec4(to_color(phi, 0.0, 1.0), u_alpha);
	}
);

/* private fuction definitions */

static void init_program()
{
	g_program = glCreateProgram();
	glueProgramAttachShaderWithSource(g_program, GL_VERTEX_SHADER,
		g_vertex_shader);
	glueProgramAttachShaderWithSource(g_program, GL_FRAGMENT_SHADER,
		g_fragment_shader);
	glBindAttribLocation(g_program, 0, "in_position");
	glBindFragDataLocation(g_program, 0, "out_frag_color");
	glueProgramLink(g_program);
	glUseProgram(g_program);
	glueProgramUniform1i(g_program, "u_quantity_tex", 0);
}

static void init_geometry()
{
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	glGenBuffers(1, &g_quad_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, g_quad_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad), g_quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

static GLuint create_texture(GLsizei width, GLsizei height)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
		GL_FLOAT, NULL);
	return tex;
}

void quantity_renderer_initialize(int cell_count_i, int cell_count_j)
{
	init_program();
	init_geometry();
	g_texture = create_texture(cell_count_i, cell_count_j);
	assert(GL_NO_ERROR == glGetError());
	g_cell_count_i = cell_count_i;
	g_cell_count_j = cell_count_j;
}

void quantity_renderer_set_alpha(float alpha)
{
	g_alpha = alpha;
}

void quantity_renderer_set_quantity_domain(float quantity_min,
	float quantity_max)
{
	g_quantity_min = quantity_min;
	g_quantity_max = quantity_max;
}

void quantity_renderer_render(const float* const q)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_cell_count_i,
		g_cell_count_j, GL_RED, GL_FLOAT, q);
	glBindVertexArray(g_vao);
	glUseProgram(g_program);
	glueProgramUniform1f(g_program, "u_alpha", g_alpha);
	glueProgramUniform1f(g_program, "u_quantity_min", g_quantity_min);
	glueProgramUniform1f(g_program, "u_quantity_max", g_quantity_max);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void quantity_renderer_finalize()
{
	glDeleteBuffers(1, &g_quad_buffer);
	glDeleteVertexArrays(1, &g_vao);
	glDeleteProgram(g_program);
	glDeleteTextures(1, &g_texture);
}
