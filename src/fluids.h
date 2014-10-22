/*
** Declares several routines useful for simulating fluids using the "Stable
** Fluids" approach. 
** 
** Some notes:
** 	- Fluid quantities fields are represented as simple float arrays
*/ 
#ifndef FLUIDS_H
#define FLUIDS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Initializes the fluid subsystem. */ 
void fluids_initialize();

/******************************************************************************
** Fluid Grid
******************************************************************************/
/* Sets the underlying grid for the fluid simulation. */ 
void fluids_set_grid(float origin_x, float origin_y, float dx, 
	int sample_count_i, int sample_count_j);

/* Samples a discrete quantity field at a point (x, y) in space. Uses bilinear
** interpolation */ 
float fluids_sample(const float* const quantities, float x, float y);

/* Creates an array for storing a discrete quantity field, sample on [grid]. 
** Initializes all samples to [c]. */ 
float* fluids_malloc(float c); 

/* Sets values of q to c */
void fluids_set(float* const q, float c);

/* sets the values of q using a function [fn] */
void fluids_set_with_function(float* const q,
	float (*fn)(float x, float y, void* const vp), void* const vp);

/******************************************************************************
** Fluid Source
******************************************************************************/
/* Applies a source [source] to a quantity field [q]. [alpha] describes the
** amount of the source added to the quantity. Each value q in [quantities] is
** incremented by [alpha] * s, where s is the source corresponding to q in
** [source]. */
void fluids_add_source(float* const q, const float* const source, float alpha);

/* Applies a source [source] to a quantity field [q]. [alpha] describes the
** amount of the source added to the quantity. Each value q in [quantities] is
** incremented by [alpha] * s, where s is the source corresponding to q in
** [source]. Each value q is bound to be [q_min] <= q <= [q_max].*/
void fluids_add_source_clamped(float* const q, const float* const source,
	float alpha, float q_min, float q_max);

/* Applies a source to a quantity field. Values of the quantity field where the
** source is greater than zero converge to a [target] value. The actual equation
** is given by:
**
** 	q += (q - [target]) * s; 
**
** where q is in [q] and s is the corresponding source value in [source]. 
** WARNING: values of s should be bound to [0, 1] */
void fluids_add_source_with_target(float* const q, const float* const source, 
	float q_target);

/******************************************************************************
** Fluid Boundary Handling
******************************************************************************/
enum {
	/* Sets the value of quantity in boundary cell to the value of the 
	** quantity of the nearest cell that is not a boundary cell. */
	FLUIDS_BOUNDARY_NN = 0,
	FLUIDS_BOUNDARY_NO_STICK_U,
	FLUIDS_BOUNDARY_NO_STICK_V,
	FLUIDS_BOUNDARY_REFLECT_U,
	FLUIDS_BOUNDARY_REFLECT_V
};

/******************************************************************************
** Fluid Advection
******************************************************************************/
/* Advects all quantities in [q_prev] according to a velocity field (u, v)
** for a time step [dt] and stores the results in [q]. [q_prev] and (u, v) are 
** assumed to be sampled on the same [grid]. */ 
void fluids_advect(float* const q, const float* const q_prev,
	const float* const u, const float* v, int boundary, float dt);

/******************************************************************************
** Fluid Diffusion
******************************************************************************/
/* Diffuses the values in [q_prev] according to some diffusion rate [diff] for
** time step [dt] and stores the result in [q]. */ 
void fluids_diffuse(float* const q, const float* const q_prev, 
	float diff, int iteration_count, int boundary, float dt);

/******************************************************************************
** Fluid Projection
******************************************************************************/
/* Makes the velocity field ([u], [v]) divergence free. [div] and [p] are 
** utility arrays that store the divergence of the velocity field before
** projection and the pressure. */ 
void fluids_project(float* const u, float* const v, int boundary_u, 
	int boundary_v, float* const p, float* const div, int iteration_count);

/******************************************************************************
** Buoyancy
******************************************************************************/
/* Applies buoyancy the v component of the velocity field [v]. The equation 
** for updating [v] is given by:
**
** 	v -= [dt] * ([alpha] * s - [beta] * (t - [temp_ambient])
**
** where v is in [v], s is a value in [smoke_dens] corresponding to v and t
** is a value in [temperatures] corresponding to v. */
void fluids_add_buoyancy(float* const v, const float* const smoke_dens, 
	const float* const temperatures, float alpha, float beta, 
	float temp_ambient, float dt);


/******************************************************************************
** Statistics
******************************************************************************/
/* Get the average divergence across the velocity field ([u], [v]). Stores the
** divergence per grid cell in [div]. */
float fluids_get_max_divergence(const float* const u, const float* const v,
	float* const div);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: FLUIDS_H */
