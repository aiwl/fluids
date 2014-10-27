/* renders particles */
#ifndef PARTICLE_RENDERER_H
#define PARTICLE_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Initializes the particle renderer. [x_min], [y_min], [x_max] and [y_max] 
** denote the region the fluid particles are assumed to be in. */
void particle_renderer_initialize(float x_min, float y_min, float x_max, 
	float y_max);

/* Renders the first [count] [positions] of the particle system. */
void particle_renderer_render(const float* const positions,
	const float* const lifetimes, unsigned int count);

/* cleans up once the particle renderer is done. */
void particle_renderer_finalize();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: PARTICLE_RENDERER_H */
