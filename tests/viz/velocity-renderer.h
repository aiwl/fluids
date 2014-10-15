#ifndef VELOCITY_RENDERER_H
#define VELOCITY_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif

void velocity_renderer_initialize(int cell_count_i, int cell_count_j, float dx);
void velocity_renderer_render(const float* const u, const float* const v);
void velocity_renderer_set_scale(float scale);
void velocity_renderer_set_alpha(float alpha);
void velocity_renderer_set_sample_freq(int sample_freq);
void velocity_renderer_finalize();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: VELOCITY-RENDERER_H */
