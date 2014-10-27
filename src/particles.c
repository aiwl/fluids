#include "particles.h"
#include "fluids.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

static float* g_positions = NULL;
static float* g_lifetimes = NULL;
static float g_particle_lifetime = 1.0;		/* particle lifetime in sec */
static unsigned int g_particle_count = 0;
static unsigned int g_particle_capacity = 0;
static unsigned int g_start_index = 0;		/* reference to the beginning
						** of a particles quantity array
						*/

static float g_emitter_x = 0.0;
static float g_emitter_y = 0.0;
static float g_emitter_r = 1.0;

/* reallocates positions for [n] new particles if needed. */
static void particles_realloc(unsigned int n)
{
	if (!g_positions) {
		g_positions = calloc(n, 2 * n * sizeof(*g_positions));
		g_lifetimes = calloc(n, sizeof(*g_lifetimes));
		g_particle_capacity = n;
	}
	
	while ((g_particle_count + n) > g_particle_capacity) {
		g_particle_capacity *= 2;
		g_positions = realloc(g_positions,
			sizeof(*g_positions) * 2 * g_particle_capacity);
		g_lifetimes = realloc(g_lifetimes,
			g_particle_capacity * sizeof(*g_lifetimes));
	}
	
	assert(g_positions);
	assert(g_lifetimes);
	return;
}

void particles_initialize()
{

}

void particles_set_emitter(float x, float y, float r)
{
	g_emitter_x = x;
	g_emitter_y = y;
	g_emitter_r = r;
}

void particles_set_lifetime(float lifetime)
{
	g_particle_lifetime = lifetime;
}

void particles_emit(unsigned int particle_count)
{
	float dx = sqrtf(g_emitter_r * g_emitter_r / particle_count);
	float x0 = g_emitter_x;
	float y0 = g_emitter_y;
	float x, y;
	unsigned int i = 0, j = 0;
	unsigned int max = ceilf(2.0 * g_emitter_r / dx);
	unsigned int idx = 0;
	
	particles_realloc(max * max);
	
	for (i = 0; i < max; i++) {
		for (j = 0; j < max; j++) {
			idx = (g_start_index + g_particle_count) %
				g_particle_capacity;
			x = i * dx - g_emitter_r;
			y = j * dx - g_emitter_r;
			
			if ((x * x + y * y) <= (g_emitter_r * g_emitter_r)) {
				g_positions[2 * idx + 0] = x0 + x;
				g_positions[2 * idx + 1] = y0 + y;
				g_lifetimes[idx] = g_particle_lifetime;
				g_particle_count++;
			}
		}
	}
}

float* particles_get_positions()
{
	return g_positions;
}

float* particles_get_lifetimes()
{
	return g_lifetimes;
}

unsigned int particles_get_count()
{
	return g_particle_capacity;
}

void particles_advect(const float* const u, const float* const v, float dt)
{
	unsigned int i = 0;
	float vel_x, vel_y;
	unsigned int idx = 0;
	unsigned int si = g_start_index;
	
	for (i = 0; i < g_particle_count; i++) {
		idx = (si + i) % g_particle_capacity;
		
		vel_x = fluids_sample(u, g_positions[2 *idx + 0],
			g_positions[2 * idx + 1]);
		vel_y = fluids_sample(v, g_positions[2 * idx + 0],
			g_positions[2 * idx + 1]);
		g_positions[2 * idx + 0] += vel_x * dt;
		g_positions[2 * idx + 1] += vel_y * dt;
		g_lifetimes[idx] -= dt;
		
		if (g_lifetimes[idx] < 0.0) {
			g_start_index = (g_start_index + 1) %
				g_particle_capacity;
			g_particle_count--;
		}
	}
}

void particles_finalize()
{
	free(g_positions);
	free(g_lifetimes);
}