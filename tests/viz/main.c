#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "quantity-renderer.h"
#include "velocity-renderer.h"
#include "../../src/fluids.h"

/* fluid domain declarations */
static float g_origin_x = -1.0;
static float g_origin_y = -1.0;
static float g_dx = 0.01;
static int g_cell_count_i = 200;
static int g_cell_count_j = 200;

/* fluid quantities */
static float* g_temperatures[2];
static float* g_us[2];
static float* g_vs[2];
static float* g_pressures;
static float* g_vel_divs;

/* quantity sources */
static float* g_temp_source;

/* quantity initializer functions */
static float set_us(float x, float y, void* const vp)
{
	return y;
}

static float set_vs(float x, float y, void* const vp)
{
	return -x;
}

static float set_temp(float x, float y, void* const vp)
{
	float r = 0.1;
	
	if (x * x + (y + 0.5) * (y + 0.5) <= r * r) {
		return 1.0;
	}
	
	return 0.0;
}

static float set_temp_src(float x, float y, void* const vp)
{
	float r = 0.1;
	
	if (x * x + (y + 0.5) * (y + 0.5) <= r * r) {
		return 1.0;
	}
	
	return 0.0;
}

void swap(float** q)
{
	float* tmp = *q;
	*(q) = *(q + 1);
	*(q + 1) = tmp;
}

static void initialize()
{
	/* init fluids and set grid */
	fluids_initialize();
	fluids_set_grid(g_origin_x, g_origin_y, g_dx, g_cell_count_i,
		g_cell_count_j);

	/* init fluid quantities */
	g_temperatures[0] = fluids_malloc(0.0);
	g_temperatures[1] = fluids_malloc(0.0);
	g_us[0] = fluids_malloc(1.0);
	g_us[1] = fluids_malloc(1.0);
	g_vs[0] = fluids_malloc(1.0);
	g_vs[1] = fluids_malloc(1.0);
	g_pressures = fluids_malloc(0.0);
	g_vel_divs = fluids_malloc(0.0);

	fluids_set_with_function(g_us[0], set_us, NULL);
	fluids_set_with_function(g_us[1], set_us, NULL);
	fluids_set_with_function(g_vs[0], set_vs, NULL);
	fluids_set_with_function(g_vs[1], set_vs, NULL);
//	fluids_set_with_function(g_temperatures[0], set_temp, NULL);
//	fluids_set_with_function(g_temperatures[1], set_temp, NULL);
	
	/* init sources */
	g_temp_source = fluids_malloc(0);
	fluids_set_with_function(g_temp_source, set_temp_src, NULL);

	
	/* init renderer */
	quantity_renderer_initialize(g_cell_count_i, g_cell_count_j);
	velocity_renderer_initialize(g_cell_count_i, g_cell_count_j, g_dx);
	velocity_renderer_set_sample_freq(4);
	velocity_renderer_set_alpha(0.5);
	velocity_renderer_set_scale(0.025);
}

static void update()
{
	fluids_add_source(g_temperatures[0], g_temp_source, 0.1);
	swap(g_temperatures);
	fluids_advect(g_temperatures[0], g_temperatures[1], g_us[0], g_vs[0],
		FLUIDS_BOUNDARY_NN, 0.02);
	swap(g_temperatures);
	fluids_diffuse(g_temperatures[0], g_temperatures[1], 0.8, 20,
		FLUIDS_BOUNDARY_NN, 0.02);
	
	fluids_project(g_us[0], g_vs[0], FLUIDS_BOUNDARY_NO_STICK_U,
		FLUIDS_BOUNDARY_NO_STICK_V, g_pressures, g_vel_divs, 20);
	
	/* render quantities and velocity */
	quantity_renderer_render(g_temperatures[0]);
	velocity_renderer_render(g_us[0], g_vs[0]);
}

static void finalize()
{
	free(g_temperatures[0]);
	free(g_temperatures[1]);
	free(g_us[0]);
	free(g_us[1]);
	free(g_vs[0]);
	free(g_vs[1]);
	quantity_renderer_finalize();
	velocity_renderer_finalize();
}

void on_click(GLFWwindow* window, int button, int action, int mods)
{

}

void on_move(GLFWwindow* window, double x, double y)
{

}


int main(void)
{
	GLFWwindow* window;
	
	/* Initialize the library */
	if (!glfwInit()) {
		return -1;
	}
	
	/* Set window hints here */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 800, "Stable Fluids", NULL, NULL);
	
	if (!window) {
		glfwTerminate();
		return -1;
	}
	
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	
	/* initialize glew */
	glewExperimental = GL_TRUE;
	assert(GLEW_OK == glewInit());
	glGetError();
	
	printf("%s\n", glGetString(GL_VERSION));
	
	
	glClearColor(1.0, 1.0, 1.0, 1.0);
	
	glfwSetMouseButtonCallback(window, on_click);
	glfwSetCursorPosCallback(window, on_move);
	
	initialize();
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		update();
	
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		
		/* Poll for and process events */
		glfwPollEvents();
	}
	
	finalize();
	
	glfwTerminate();
	return 0;
}