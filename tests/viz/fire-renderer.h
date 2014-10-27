#ifndef FIRE_RENDERER_H
#define FIRE_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif

void fire_renderer_initialize(int cell_count_i, int cell_count_j);
void fire_renderer_set_alpha_spec(float (*spec)(float x));
void fire_renderer_set_temperature_bounds(float min, float max);
void fire_renderer_render(const float* const smoke_densities,
	const float* const temperatures);
void fire_renderer_finalize();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: FIRE-RENDERER_H */
