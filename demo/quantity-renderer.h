#ifndef QUANTITY_RENDERER_H
#define QUANTITY_RENDERER_H

void quantity_renderer_initialize(int cell_count_i, int cell_count_j);
void quantity_renderer_render(const float* const q);
void quantity_renderer_set_alpha(float alpha);
void quantity_renderer_set_quantity_domain(float quantity_min,
	float quantity_max);
void quantity_renderer_finalize();

#endif /* end of include guard: QUANTITY_RENDERER_H */
