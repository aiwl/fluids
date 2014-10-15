#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "quantity-renderer.h"
#include "velocity-renderer.h"
#include "../../src/fluids.h"

/* window settings */
static int g_window_width = 800;
static int g_window_height = 800;

/* fluid domain declarations */
static float g_origin_x = -1.0;
static float g_origin_y = -1.0;
static float g_dx = 0.01;
static int g_cell_count_i = 200;
static int g_cell_count_j = 200;

/* fluid quantities */
static float* g_temperatures[2];
static float* g_smoke_densities[2];
static float* g_us[2];
static float* g_vs[2];
static float* g_pressures;
static float* g_vel_divs;

/* quantity sources */
static float* g_temp_source;

/* simulation constants */
static const float g_temp_ambient = 273.3;
static const float g_temp_target = 973.3;
static const float g_dt = 0.04;

/* user input */
static int g_is_clicked = 0;
static float g_cursor_x = 400;
static float g_cursor_y = 400;


/* quantity initializer functions */
static float set_temp_src(float x, float y, void* const vp)
{
	float r = 0.15;
	float x0 = g_origin_x + (g_cursor_x / g_window_width) * g_cell_count_i * g_dx;
	float y0 = g_origin_y + (g_cursor_y / g_window_height) * g_cell_count_j * g_dx;
	
	if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= r * r) {
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
	g_temperatures[0] = fluids_malloc(g_temp_ambient);
	g_temperatures[1] = fluids_malloc(g_temp_ambient);
	g_smoke_densities[0] = fluids_malloc(0.0);
	g_smoke_densities[1] = fluids_malloc(0.0);
	g_us[0] = fluids_malloc(0.0);
	g_us[1] = fluids_malloc(0.0);
	g_vs[0] = fluids_malloc(0.0);
	g_vs[1] = fluids_malloc(0.0);
	g_pressures = fluids_malloc(0.0);
	g_vel_divs = fluids_malloc(0.0);
	
	/* init sources */
	g_temp_source = fluids_malloc(0);
	fluids_set_with_function(g_temp_source, set_temp_src, NULL);
		
	/* init renderer */
	quantity_renderer_initialize(g_cell_count_i, g_cell_count_j);
	quantity_renderer_set_quantity_domain(g_temp_ambient, g_temp_target);
	velocity_renderer_initialize(g_cell_count_i, g_cell_count_j, g_dx);
	velocity_renderer_set_sample_freq(4);
	velocity_renderer_set_alpha(0.5);
	velocity_renderer_set_scale(0.05);
}

static void do_vel_step()
{
	/* update velocities */
	fluids_add_buoyancy(g_vs[0], g_smoke_densities[0], g_temperatures[0],
		0.0, 0.0025, g_temp_ambient, g_dt);
	swap(g_us);
	fluids_diffuse(g_us[0], g_us[1], 0.5, 20, FLUIDS_BOUNDARY_REFLECT_U,
		g_dt);
	swap(g_vs);
	fluids_diffuse(g_vs[0], g_vs[1], 0.5, 20, FLUIDS_BOUNDARY_REFLECT_V,
		g_dt);
	swap(g_us);
	swap(g_vs);
	fluids_advect(g_us[0], g_us[1] , g_us[1], g_vs[1],
		FLUIDS_BOUNDARY_REFLECT_U, g_dt);
	fluids_advect(g_vs[0], g_vs[1] , g_us[1], g_vs[1],
		FLUIDS_BOUNDARY_REFLECT_V, g_dt);
	fluids_project(g_us[0], g_vs[0], FLUIDS_BOUNDARY_REFLECT_U,
		FLUIDS_BOUNDARY_REFLECT_V, g_pressures, g_vel_divs, 100);
}

static void do_temp_step()
{
	if (g_is_clicked) {
		fluids_add_source_with_target(g_temperatures[0], g_temp_source,
			g_temp_target);
	}

	swap(g_temperatures);
	fluids_advect(g_temperatures[0], g_temperatures[1], g_us[0], g_vs[0],
		FLUIDS_BOUNDARY_NN, g_dt);
	swap(g_temperatures);
	fluids_diffuse(g_temperatures[0], g_temperatures[1], 10.8, 20,
		FLUIDS_BOUNDARY_NN, g_dt);
}

static void update()
{
	do_vel_step();
	do_temp_step();
		
	/* render quantities and velocity */
	quantity_renderer_render(g_temperatures[0]);
	velocity_renderer_render(g_us[0], g_vs[0]);
	
	printf("%f\n", fluids_sample(g_temperatures[0], -0.1, -0.5));
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

static void update_temp_source()
{
	fluids_set(g_temp_source, 0.0);
	fluids_set_with_function(g_temp_source, set_temp_src, NULL);
}

void on_click(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;

	if (action == GLFW_PRESS) {
		glfwGetCursorPos(window, &x, &y);
		g_cursor_x = x;
		g_cursor_y = g_window_height - y;
		g_is_clicked = 1;
		update_temp_source();
	} else {
		g_is_clicked = 0;
	}
}

void on_move(GLFWwindow* window, double x, double y)
{
	if (g_is_clicked) {
		g_cursor_x = x;
		g_cursor_y = g_window_height - y;
		update_temp_source();
	}
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
	window = glfwCreateWindow(g_window_width, g_window_height,
		"Stable Fluids", NULL, NULL);
	
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