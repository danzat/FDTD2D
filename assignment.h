#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include "fdtd2d.h"
#include <cairo.h>

typedef long (*iterate_func_t)(fdtd_2d_t*);
typedef void (*render_func_t)(cairo_t*, fdtd_2d_t*);

long iterate_section_3(fdtd_2d_t *fdtd);
long iterate_section_4(fdtd_2d_t *fdtd);
long iterate_section_5(fdtd_2d_t *fdtd);
long iterate_section_6(fdtd_2d_t *fdtd);
long iterate_section_7(fdtd_2d_t *fdtd);
long iterate_section_8(fdtd_2d_t *fdtd);
long iterate_section_9(fdtd_2d_t *fdtd);

void render_regular(cairo_t *c, fdtd_2d_t *fdtd);
void render_with_obstacle(cairo_t *c, fdtd_2d_t *fdtd);

static const iterate_func_t iterate_funcs[] = {
    [3] = iterate_section_3,
    [4] = iterate_section_4,
    [5] = iterate_section_5,
    [6] = iterate_section_6,
    [7] = iterate_section_7,
    [8] = iterate_section_8,
    [9] = iterate_section_9
};

static const render_func_t render_funcs[] = {
    [3] = render_regular,
    [4] = render_regular,
    [5] = render_with_obstacle,
    [6] = render_regular,
    [7] = render_regular,
    [8] = render_with_obstacle,
    [9] = render_with_obstacle
};

/* helper functions */
void copy_simulation_up_to(fdtd_2d_t *fdtd, long T);
void place_PEC_obstacle(fdtd_2d_t *fdtd, long center_x, long center_y, long side);
#endif
