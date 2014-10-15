#include "velocity-renderer.h"
#include "program.h"
#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#endif

/* shader */

#define TO_STRING(x) #x
const char* g_vertex_shader = "#version 150\n"
TO_STRING(
	in float in_u; // velocities u component
	in float in_v; // velocities v component
	
	uniform int u_grid_width;
	uniform int u_grid_height;
	uniform int u_sample_number;
	
	uniform float u_scale;
	uniform float u_dx;
	
	flat out int v_is_discarded;
	out vec2 v_pos2; // end position of the line
	
	void main()
	{
		gl_PointSize = 1;
		int id = gl_VertexID;
		int j = id / u_grid_width;
		int i = int(mod(id, u_grid_width));
		
		float w = u_grid_width * u_dx;
		float h = u_grid_height * u_dx;
				
		float x = (float(i) + 0.5) * u_dx;
		float y = (float(j) + 0.5) * u_dx;

		v_pos2.x = x + in_u * u_scale;
		v_pos2.y = y + in_v * u_scale;
		
		/* check is this grid sample should be disregarded */
		if (i % u_sample_number == 0 && j % u_sample_number == 0) {
			v_is_discarded = 0;
		} else {
			v_is_discarded = 1;
		}
		
		// convert points to NDC - space
		x = 2.0 * x / w - 1.0;
		y = 2.0 * y / h - 1.0;
		
		v_pos2.x = 2.0 * v_pos2.x / w - 1.0;
		v_pos2.y = 2.0 * v_pos2.y / h - 1.0;
			
		gl_Position = vec4(x, y, 0.0, 1.0);
	}
);

const char* g_geometry_shader = "#version 150\n"
TO_STRING(
	layout (points) in;
	layout (line_strip, max_vertices = 2) out;
	
	flat in int v_is_discarded[1];
	in vec2 v_pos2[1];
	
	flat out int g_is_discarded;
	
	void main()
	{
		g_is_discarded = v_is_discarded[0];
		gl_Position = gl_in[0].gl_Position;
		EmitVertex();

		//g_is_discarded = v_is_discarded[0];
		gl_Position = vec4(v_pos2[0], 0.0, 1.0);
		EmitVertex();
	}
);

const char* g_fragment_shader = "#version 150\n"
TO_STRING(
	flat in int g_is_discarded;
	
	uniform float u_alpha;
	
	out vec4 out_frag_color;
	
	void main()
	{
		if (g_is_discarded == 1) {
			discard;
		}
	
		out_frag_color = vec4(0.0, 0.0, 0.0, u_alpha);
	}
);

/* file private variables */

static GLuint g_program = 0;
static GLuint g_vao = 0;
static GLuint g_u_buffer = 0;
static GLuint g_v_buffer = 0;
static GLsizeiptr g_buffer_size = 0;
static int g_cell_count_i = 0;
static int g_cell_count_j = 0;
static GLint g_sample_freq = 2;
static GLfloat g_alpha = 0.5;
static GLfloat g_dx = 0.0;
static GLfloat g_scale = 1.0;


static void init_program()
{
	g_program = glCreateProgram();
	glueProgramAttachShaderWithSource(g_program, GL_VERTEX_SHADER,
		g_vertex_shader);
	glueProgramAttachShaderWithSource(g_program, GL_GEOMETRY_SHADER,
		g_geometry_shader);
	glueProgramAttachShaderWithSource(g_program, GL_FRAGMENT_SHADER,
		g_fragment_shader);
	glBindAttribLocation(g_program, 0, "in_u");
	glBindAttribLocation(g_program, 1, "in_v");
	glBindFragDataLocation(g_program, 0, "out_frag_color");
	glueProgramLink(g_program);
	assert(GL_NO_ERROR == glGetError());
}

static void init_geometry(GLuint* vao, GLuint* u_buffer, GLuint* v_buffer,
	GLsizeiptr size)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
	glGenBuffers(1, u_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *u_buffer);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, v_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *v_buffer);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
}

void velocity_renderer_initialize(int cell_count_i, int cell_count_j, float dx)
{
	init_program();
	g_buffer_size = cell_count_i * cell_count_j * sizeof(float);
	init_geometry(&g_vao, &g_u_buffer, &g_v_buffer, g_buffer_size);
	assert(GL_NO_ERROR == glGetError());
	g_cell_count_i = cell_count_i;
	g_cell_count_j = cell_count_j;
	g_dx = dx;
}

void velocity_renderer_render(const float* const u, const float* const v)
{
	glBindBuffer(GL_ARRAY_BUFFER, g_u_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, g_buffer_size, u);
	glBindBuffer(GL_ARRAY_BUFFER, g_v_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, g_buffer_size, v);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glUseProgram(g_program);
	glueProgramUniform1i(g_program, "u_grid_width", g_cell_count_i);
	glueProgramUniform1i(g_program, "u_grid_height", g_cell_count_j);
	glueProgramUniform1f(g_program, "u_alpha", g_alpha);
	glueProgramUniform1i(g_program, "u_sample_number", g_sample_freq);
	glueProgramUniform1f(g_program, "u_scale", g_scale);
	glueProgramUniform1f(g_program, "u_dx", g_dx);
	glBindVertexArray(g_vao);
	glDrawArrays(GL_POINTS, 0, (GLsizei)(g_buffer_size / sizeof(float)));
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void velocity_renderer_set_scale(float scale)
{
	g_scale = scale;
}

void velocity_renderer_set_alpha(float alpha)
{
	g_alpha = alpha;
}

void velocity_renderer_set_sample_freq(int sample_freq)
{
	g_sample_freq = sample_freq;
}


void velocity_renderer_finalize()
{
	glDeleteProgram(g_program);
	glDeleteVertexArrays(1, &g_vao);
	glDeleteBuffers(1, &g_u_buffer);
	glDeleteBuffers(1, &g_v_buffer);
}
