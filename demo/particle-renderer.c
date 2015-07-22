#include "particle-renderer.h"
#include "program.h"
#include <assert.h>

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#endif

/******************************************************************************
** Shader Code
******************************************************************************/
#define TO_STRING(x) #x
static const char* g_vertex_shader =
	"#version 150\n"
TO_STRING(
	uniform float u_x_min;
	uniform float u_y_min;
	uniform float u_x_max;
	uniform float u_y_max;
	in vec2 v_position;
	in float v_lifetime;
	out float vo_lifetime;

	void main()
	{
		vo_lifetime = v_lifetime;
		float ext_x = u_x_max - u_x_min;
		float ext_y = u_y_max - u_y_min;
		float x_ndc = 2.0 * (v_position.x - u_x_min) / ext_x - 1.0;
		float y_ndc = 2.0 * (v_position.y - u_y_min) / ext_y - 1.0;
		gl_PointSize = 2.0;
		gl_Position = vec4(x_ndc, y_ndc, 0.0, 1.0);
	}
);

static const char* g_fragment_shader =
	"#version 150\n"
TO_STRING(
	in float vo_lifetime;
	out vec4 f_fragment_color;
	
	void main()
	{
		if (vo_lifetime <= 0.0) {
			discard;
		}
	
		f_fragment_color = vec4(1.0, 0.0, 0.0, 1.0);
	}
);

/******************************************************************************
** File private definitions
******************************************************************************/
static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_positions_buffer = 0;
static GLuint g_lifetime_buffer = 0;
static GLuint g_particle_capacity = 0;

void init_program()
{
	g_program = glCreateProgram();
	glueProgramAttachShaderWithSource(g_program, GL_VERTEX_SHADER,
		g_vertex_shader);
	glueProgramAttachShaderWithSource(g_program, GL_FRAGMENT_SHADER,
		g_fragment_shader);
	glBindAttribLocation(g_program, 0, "v_position");
	glBindAttribLocation(g_program, 1, "v_lifetime");
	glBindFragDataLocation(g_program, 0, "f_fragment_color");
	glueProgramLink(g_program);
	assert(GL_NO_ERROR == glGetError());
}

void init_geometry()
{
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	glGenBuffers(1, &g_positions_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, g_positions_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &g_lifetime_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, g_lifetime_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
	assert(GL_NO_ERROR == glGetError());
}

/******************************************************************************
** Particle Renderer
******************************************************************************/
void particle_renderer_initialize(float x_min, float y_min, float x_max, 
	float y_max)
{
	init_program();
	init_geometry();
	
	glUseProgram(g_program);
	glueProgramUniform1f(g_program, "u_x_min", x_min);
	glueProgramUniform1f(g_program, "u_y_min", y_min);
	glueProgramUniform1f(g_program, "u_x_max", x_max);
	glueProgramUniform1f(g_program, "u_y_max", y_max);

}

void particle_renderer_render(const float* const positions,
	const float* const lifetimes, unsigned int count)
{
	/* copy particle positions to the buffer */
	if (count > g_particle_capacity) {
		glBindBuffer(GL_ARRAY_BUFFER, g_positions_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(*positions) * 2 * count,
			positions, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, g_lifetime_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(*lifetimes) * count,
			lifetimes, GL_DYNAMIC_DRAW);
		g_particle_capacity = count;
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, g_positions_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0,
			sizeof(*positions) * 2 * count, positions);
		glBindBuffer(GL_ARRAY_BUFFER, g_lifetime_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0,
			sizeof(*lifetimes) * count, lifetimes);

	}
	
	/* render positions */
	glEnable(GL_PROGRAM_POINT_SIZE);
	glBindVertexArray(g_vao);
	glUseProgram(g_program);
	glDrawArrays(GL_POINTS, 0, count);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void particle_renderer_finalize()
{
	glDeleteBuffers(1, &g_positions_buffer);
	glDeleteVertexArrays(1, &g_vao);
	glDeleteProgram(g_program);
}

