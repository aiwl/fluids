#include "fluids.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>


/* a small float. mainly to avoid division by zero */
#define EPS FLT_MIN

/* grid */

static float g_origin_x = 0.0;
static float g_origin_y = 0.0;
static float g_dx = 0.01;
static int g_cell_count_i = 100;
static int g_cell_count_j = 100;

#define IDX(i, j) g_cell_count_i * (j) + (i)
#define CLAMP(i, j) 						\
	i = i < 0 ? 0 : i; 					\
	i = i >= g_cell_count_i ? g_cell_count_i - 1 : i; 	\
	j = j < 0 ? 0 : j; 					\
	j = j >= g_cell_count_j ? g_cell_count_j - 1 : j;

/* boundary handler definitions */

static void set_boundary_nn(float* const q)
{
	int i = 0, j = 0;	

	for (i = 1; i < g_cell_count_i - 1; i++) {
		q[IDX(i, 0)] = q[IDX(i, 1)];
		q[IDX(i, g_cell_count_j - 1)] = q[IDX(i, g_cell_count_j - 2)];
	}

	for (j = 1; j < g_cell_count_j - 1; j++) {
		q[IDX(0, j)] = q[IDX(1, j)];
		q[IDX(g_cell_count_i - 1, j)] = 
			q[IDX(g_cell_count_i - 2, j)];
	}

	q[IDX(0, 0)] = 0.5 * (q[IDX(0, 1)] + q[IDX(1, 0)]);
	q[IDX(g_cell_count_i - 1, 0)] = 0.5 * (q[IDX(g_cell_count_i - 1, 1)] + 
		q[IDX(g_cell_count_i - 2, 0)]);
	q[IDX(0, g_cell_count_j - 1)] = 0.5 * (q[IDX(0, g_cell_count_j - 2)] + 
		q[IDX(1, g_cell_count_j - 1)]);
	q[IDX(g_cell_count_i - 1, g_cell_count_j - 1)] = 0.5 * (
		q[IDX(g_cell_count_i - 2, g_cell_count_j - 1)] + 
		q[IDX(g_cell_count_i - 1, g_cell_count_j - 2)]);
}

static void set_boundary_no_stick_u(float* const q)
{
	int i = 0, j = 0;	

	for (i = 1; i < g_cell_count_i - 1; i++) {
		q[IDX(i, 0)] = q[IDX(i, 1)];
		q[IDX(i, g_cell_count_j - 1)] = q[IDX(i, g_cell_count_j - 2)];
	}

	for (j = 1; j < g_cell_count_j - 1; j++) {
		q[IDX(0, j)] = 0.0; 
		q[IDX(g_cell_count_i - 1, j)] = 0.0;
	}

	q[IDX(0, 0)] = 0.5 * (q[IDX(0, 1)] + q[IDX(1, 0)]);
	q[IDX(g_cell_count_i - 1, 0)] = 0.5 * (q[IDX(g_cell_count_i - 1, 1)] + 
		q[IDX(g_cell_count_i - 2, 0)]);
	q[IDX(0, g_cell_count_j - 1)] = 0.5 * (q[IDX(0, g_cell_count_j - 2)] + 
		q[IDX(1, g_cell_count_j - 1)]);
	q[IDX(g_cell_count_i - 1, g_cell_count_j - 1)] = 0.5 * (
		q[IDX(g_cell_count_i - 2, g_cell_count_j - 1)] + 
		q[IDX(g_cell_count_i - 1, g_cell_count_j - 2)]);
}

static void set_boundary_reflect_u(float* const q)
{
	int i = 0, j = 0;	

	for (i = 1; i < g_cell_count_i - 1; i++) {
		q[IDX(i, 0)] = q[IDX(i, 1)];
		q[IDX(i, g_cell_count_j - 1)] = q[IDX(i, g_cell_count_j - 2)];
	}

	for (j = 1; j < g_cell_count_j - 1; j++) {
		q[IDX(0, j)] = -q[IDX(1, j)];
		q[IDX(g_cell_count_i - 1, j)] = 
			-q[IDX(g_cell_count_i - 2, j)];
	}

	q[IDX(0, 0)] = 0.5 * (q[IDX(0, 1)] + q[IDX(1, 0)]);
	q[IDX(g_cell_count_i - 1, 0)] = 0.5 * (q[IDX(g_cell_count_i - 1, 1)] + 
		q[IDX(g_cell_count_i - 2, 0)]);
	q[IDX(0, g_cell_count_j - 1)] = 0.5 * (q[IDX(0, g_cell_count_j - 2)] + 
		q[IDX(1, g_cell_count_j - 1)]);
	q[IDX(g_cell_count_i - 1, g_cell_count_j - 1)] = 0.5 * (
		q[IDX(g_cell_count_i - 2, g_cell_count_j - 1)] + 
		q[IDX(g_cell_count_i - 1, g_cell_count_j - 2)]);
}

static void set_boundary_no_stick_v(float* const q)
{
	int i = 0, j = 0;	

	for (i = 1; i < g_cell_count_i - 1; i++) {
		q[IDX(i, 0)] = 0.0;
		q[IDX(i, g_cell_count_j - 1)] = 0.0; 
	}

	for (j = 1; j < g_cell_count_j - 1; j++) {
		q[IDX(0, j)] = q[IDX(1, j)];
		q[IDX(g_cell_count_i - 1, j)] = 
			q[IDX(g_cell_count_i - 2, j)];
	}

	q[IDX(0, 0)] = 0.5 * (q[IDX(0, 1)] + q[IDX(1, 0)]);
	q[IDX(g_cell_count_i - 1, 0)] = 0.5 * (q[IDX(g_cell_count_i - 1, 1)] + 
		q[IDX(g_cell_count_i - 2, 0)]);
	q[IDX(0, g_cell_count_j - 1)] = 0.5 * (q[IDX(0, g_cell_count_j - 2)] + 
		q[IDX(1, g_cell_count_j - 1)]);
	q[IDX(g_cell_count_i - 1, g_cell_count_j - 1)] = 0.5 * (
		q[IDX(g_cell_count_i - 2, g_cell_count_j - 1)] + 
		q[IDX(g_cell_count_i - 1, g_cell_count_j - 2)]);
}

static void set_boundary_reflect_v(float* const q)
{
	int i = 0, j = 0;	

	for (i = 1; i < g_cell_count_i - 1; i++) {
		q[IDX(i, 0)] = -q[IDX(i, 1)];
		q[IDX(i, g_cell_count_j - 1)] = -q[IDX(i, g_cell_count_j - 2)];
	}

	for (j = 1; j < g_cell_count_j - 1; j++) {
		q[IDX(0, j)] = q[IDX(1, j)];
		q[IDX(g_cell_count_i - 1, j)] = 
			q[IDX(g_cell_count_i - 2, j)];
	}

	q[IDX(0, 0)] = 0.5 * (q[IDX(0, 1)] + q[IDX(1, 0)]);
	q[IDX(g_cell_count_i - 1, 0)] = 0.5 * (q[IDX(g_cell_count_i - 1, 1)] + 
		q[IDX(g_cell_count_i - 2, 0)]);
	q[IDX(0, g_cell_count_j - 1)] = 0.5 * (q[IDX(0, g_cell_count_j - 2)] + 
		q[IDX(1, g_cell_count_j - 1)]);
	q[IDX(g_cell_count_i - 1, g_cell_count_j - 1)] = 0.5 * (
		q[IDX(g_cell_count_i - 2, g_cell_count_j - 1)] + 
		q[IDX(g_cell_count_i - 1, g_cell_count_j - 2)]);
}

#define BOUNDARY_HANDLER_COUNT 12
static void (*set_boundary[BOUNDARY_HANDLER_COUNT])(float* const q);

void fluids_initialize()
{
	/* set boundary handling functions */
	set_boundary[FLUIDS_BOUNDARY_NN] = set_boundary_nn;
	set_boundary[FLUIDS_BOUNDARY_NO_STICK_U] = set_boundary_no_stick_u;
	set_boundary[FLUIDS_BOUNDARY_NO_STICK_V] = set_boundary_no_stick_v;
	set_boundary[FLUIDS_BOUNDARY_REFLECT_U] = set_boundary_reflect_u;
	set_boundary[FLUIDS_BOUNDARY_REFLECT_V] = set_boundary_reflect_v;
}

void fluids_set_grid(float origin_x, float origin_y, float dx, 
	int cell_count_i, int cell_count_j)
{
	g_origin_x = origin_x;
	g_origin_y = origin_y;
	g_dx = dx;
	g_cell_count_i = cell_count_i;
	g_cell_count_j = cell_count_j;
}

float fluids_sample(const float* const quantities, float x, float y)
{
	int i, j, ip1, jp1;
	int idx_ij, idx_ip1j, idx_ijp1, idx_ip1jp1;
	float dx, dy;
	float q_i, q_i2;
	float x0 = x - g_origin_x;
	float y0 = y - g_origin_y;

	/* Compute coordinates of the grid cell (x, y) lies in */
	i = x0 / g_dx;
	j = y0 / g_dx;
	CLAMP(i, j);

	/* Compute offset of (x, y) in grid cell (i, j). The offset is relative
	** to the grid spacing [g_dx], i.e. in [0, 1] */
	dx = (x0 - i * g_dx) / g_dx;
	dy = (y0 - j * g_dx) / g_dx;
	ip1 = i + 1;
	jp1 = j + 1;
	CLAMP(ip1, jp1);

	/* get indices for coordinates */
	idx_ij = IDX(i, j);
	idx_ip1j = IDX(ip1, j);
	idx_ijp1 = IDX(i, jp1);
	idx_ip1jp1 = IDX(ip1, jp1);

	q_i = (1.0 - dx) * quantities[idx_ij] + dx * quantities[idx_ip1j];
	q_i2 = (1.0 - dx) * quantities[idx_ijp1] + dx * quantities[idx_ip1jp1];
	return (1.0 - dy) * q_i + dy * q_i2;
}


float* fluids_malloc(float c)
{
	size_t cell_count = g_cell_count_i * g_cell_count_j;
	size_t i = 0;
	float* qp = malloc(sizeof(*qp) * cell_count);	

	for (i = 0; i < cell_count; i++) {
		qp[i] = c;
	}
	
	return qp;
}

void fluids_set(float* const q, float c)
{
	size_t cell_count = g_cell_count_i * g_cell_count_j;
	int i = 0;

	for (i = 0; i < cell_count; i++) {
		q[i] = c;
	}
}

void fluids_set_with_function(float* const q,
	float (*fn)(float x, float y, void* const vp), void* const vp)
{
	int i = 0, j = 0;
	float x = 0.0, y = 0.0;

	for (j = 0; j < g_cell_count_j; j++) {
		for (i = 0; i < g_cell_count_i; i++) {
			x = g_origin_x  + i * g_dx;
			y = g_origin_y  + j * g_dx;
			q[IDX(i, j)] = (*fn)(x, y, vp);
		}
	}
}

void fluids_add_source_clamped(float* const q, const float* const source,
	float alpha, float q_min, float q_max)
{
	int i = 0, j = 0;
	int idx = 0;

	for (j = 0; j < g_cell_count_j; j++) {
		for (i = 0; i < g_cell_count_i; i++) {
			idx = IDX(i, j);	
			q[idx] += alpha * source[idx];
			
			q[idx] = q[idx] > q_max ? q_max : q[idx];
			q[idx] = q[idx] < q_min ? q_min : q[idx];
		}
	}
}
	
void fluids_add_source(float* const q, const float* const source, 
	float alpha)
{
	int i = 0, j = 0;
	int idx = 0;

	for (j = 0; j < g_cell_count_j; j++) {
		for (i = 0; i < g_cell_count_i; i++) {
			idx = IDX(i, j);	
			q[idx] += alpha * source[idx];
		}
	}
}

void fluids_add_source_with_target(float* const q, const float* const source,
	float q_target)
{
	int i = 0, j = 0;
	int idx = 0;

	for (j = 0; j < g_cell_count_j; j++) {
		for (i = 0; i < g_cell_count_i; i++) {
			idx = IDX(i, j);	
			q[idx] += (q_target - q[idx])*  source[idx];
		}
	}	
}
	
void fluids_advect(float* const q, const float* const q_prev, 
	const float* const u, const float* v, int boundary, float dt)
{
	int i = 0, j = 0;
	int idx = 0;
	float x = 0.0, y = 0.0;
	float x0 = g_origin_x;
	float y0 = g_origin_y;

	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			idx = IDX(i, j);	
			x = x0 + i * g_dx - dt * u[idx];
			y = y0 + j * g_dx - dt * v[idx];
			q[idx] = fluids_sample(q_prev, x, y);
		}
	}

	(*set_boundary[boundary])(q);
}

void fluids_diffuse(float* const q, const float* const q_prev,  float diff, 
	int iteration_count, int boundary, float dt) 
{
	int i = 0, j = 0, k = 0;
	int idx_ij, idx_ip1j, idx_im1j, idx_ijp1, idx_ijm1;
	float r = diff * dt / (g_dx * g_dx);
	float a = 1.0 / (1.0 + 4.0 * r);
	float tmp = 0.0;

	for (k = 0; k < iteration_count; k++) {
		for (j = 1; j < g_cell_count_j - 1; j++) {
			for (i = 1; i < g_cell_count_i - 1; i++) {
				idx_ij = IDX(i, j);		
				idx_ip1j = IDX(i + 1, j);		
				idx_im1j = IDX(i - 1, j);		
				idx_ijp1 = IDX(i, j + 1);
				idx_ijm1 = IDX(i, j - 1);
				tmp = q_prev[idx_ip1j] + q_prev[idx_im1j] +
					q_prev[idx_ijp1] + q_prev[idx_ijm1];
				q[idx_ij] = a * (q_prev[idx_ij] + r * tmp);
			}
		}		
	}

	(*set_boundary[boundary])(q);
}

void fluids_project(float* const u, float* const v, int boundary_u, 
	int boundary_v, float* const p, float* const div, int iteration_count)
{
	int i = 0, j = 0, k = 0;

	/* compute divergence of the velocity field and set pressure to 0 */
	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			div[IDX(i, j)] = -0.5 * g_dx * (
				u[IDX(i + 1, j)] -
				u[IDX(i - 1, j)] +
				v[IDX(i, j + 1)] -
				v[IDX(i, j - 1)]); 	
			p[IDX(i, j)] = 0.0;
		}
	}

	(*set_boundary[FLUIDS_BOUNDARY_NN])(div);
	(*set_boundary[FLUIDS_BOUNDARY_NN])(p);

	/* compute pressure */
	for (k = 0; k < iteration_count; k++) {
		for (j = 1; j < g_cell_count_j - 1; j++) {
			for (i = 1; i < g_cell_count_i - 1; i++) {
				p[IDX(i, j)] = (div[IDX(i, j)] + 
					p[IDX(i + 1, j)] +
					p[IDX(i - 1, j)] +
					p[IDX(i, j + 1)] +
					p[IDX(i, j - 1)]) / 4.0;
			}
		}
		
		(*set_boundary[FLUIDS_BOUNDARY_NN])(p);
	}

	/* substract the pressure gradient from the velocity field */
	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			u[IDX(i, j)] -= 0.5 / g_dx * (p[IDX(i + 1, j)] - 
				p[IDX(i - 1, j)]);
			v[IDX(i, j)] -= 0.5 / g_dx * (p[IDX(i, j + 1)] - 
				p[IDX(i, j - 1)]);
		}	
	}
	
	(*set_boundary[boundary_u])(u);
	(*set_boundary[boundary_v])(v);
}

void fluids_add_buoyancy(float* const v, const float* const smoke_dens, 
	const float* const temperatures, float alpha, float beta, 
	float temp_ambient, float dt)
{
	int i = 0, j = 0;

//	for (j = 1; j < g_cell_count_j - 1; j++) {
//		for (i = 1; i < g_cell_count_i - 1; i++) {
//			v[IDX(i,j)] -= dt * (alpha * smoke_dens[IDX(i,j)] -
//				beta * (temperatures[IDX(i, j)] - temp_ambient));
//		}
//	}

	for (j = 0; j < g_cell_count_j; j++) {
		for (i = 0; i < g_cell_count_i; i++) {
			v[IDX(i,j)] -= dt * (alpha * smoke_dens[IDX(i,j)] -
				beta * (temperatures[IDX(i, j)] - temp_ambient));
		}
	}
}

void fluids_add_vorticity_confinement(float* const u, float* const v,
	float* const vorticity, float* const nvg_x, float* const nvg_y,
	float eps, float dt)
{
	int i = 0, j = 0;
	float gx, gy, gm;
	float a = 1.0 / (2.0 * g_dx);
	float b = eps * dt * g_dx;

	/* compute vorticity */
	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			vorticity[IDX(i, j)] =
				a * ((v[IDX(i + 1, j)] - v[IDX(i - 1, j)]) -
				(u[IDX(i, j + 1)] - u[IDX(i, j - 1)]));
		}
	}
	
	printf("vort: %f == %f\n", vorticity[IDX(80, 50)], vorticity[IDX(20, 50)]);
	
	/* compute normalized vorticity gradient */
	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			gx = a * (fabs(vorticity[IDX(i + 1, j)]) -
				fabs(vorticity[IDX(i - 1, j)]));
			gy = a * (fabs(vorticity[IDX(i, j + 1)]) -
				fabs(vorticity[IDX(i, j - 1)]));
			gm = sqrtf(gx * gx + gy * gy);
			nvg_x[IDX(i, j)] = gx / (gm + EPS);
			nvg_y[IDX(i, j)] = gy / (gm + EPS);
		}
	}

	/* add contribution of vorticity confinement to the velocity */
	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			u[IDX(i, j)] += b * vorticity[IDX(i, j)] *
				nvg_y[IDX(i, j)];
			v[IDX(i, j)] -= b * vorticity[IDX(i, j)] *
				nvg_x[IDX(i, j)];
//				
//			if (nvg_x[IDX(i, j)] < 0.0) {
//				puts("negative");
//			}
		}
	}
}

float fluids_get_max_divergence(const float* const u, const float* const v,
	float* const div)
{
	int i = 0, j = 0;
	float avg_div = 0.0;

	/* compute divergence of the velocity field and set pressure to 0 */
	for (j = 1; j < g_cell_count_j - 1; j++) {
		for (i = 1; i < g_cell_count_i - 1; i++) {
			div[IDX(i, j)] =  (
				u[IDX(i + 1, j)] -
				u[IDX(i - 1, j)] +
				v[IDX(i, j + 1)] -
				v[IDX(i, j - 1)]) / (2.0 * g_dx);
				
			avg_div = div[IDX(i, j)] > avg_div ? div[IDX(i, j)] : avg_div;
		}
	}
	
	return avg_div;
}

