/* particles to be advected by the fluid */ 
#ifndef PARTICLES_H
#define PARTICLES_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Initializes the particle subsystem. */
void particles_initialize();

/* Set the emitter region. Particles are seeded within a circle with origin 
** ([x], [y]) and radius [r]. */
void particles_set_emitter(float x, float y, float r);

/* Sets the lifetime a particles initialized with. */
void particles_set_lifetime(float lifetime);

/* Emits approx. [particle_count] particles within the emitter region. */
void particles_emit(unsigned int particle_count);

/* Gets the positions of the particles. Particle positons in both dimensions are
** tightly packed in a float array. */
float* particles_get_positions();

/* Gets the lifetime for each particle */
float* particles_get_lifetimes();

/* Get the current amount of particles in the system. */
unsigned int particles_get_count();

/* Advects particles according to an underlying velocity field ([u], [v]) for
** a timestep [dt]. */
void particles_advect(const float* const u, const float* const v, float dt);

/* Cleans up, when the particle subsystem is done. */
void particles_finalize();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: PARTICLES_H */
