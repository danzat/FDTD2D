#include "excitation.h"
#include "assignment.h"
#include "fdtd2d.h"
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>

long iterate_section_3(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    /* Excitation on the surface i = 0 */
    for (j = 0; j < J; ++j) {
        E_Y(0, j) = fdtd_excitation(n, N_P) * 377.0;
    }
    /* Regular solution for 0<i<I */
    for (i = 1; i < I; ++i) {
        for (j = 0; j < J; ++j) {
            E_Y(i, j) += -alpha_Ey * (H_Z(i, j) - H_Z(i-1, j));
        }
    }
    /* PEC at the surface i = I */
    for (j = 0; j < J; ++j) {
        E_Y(I, j) = 0;
    }
    /* Ex and Hz are calculated regularly */
    FDTD_2D_iterate_Ex(fdtd);
    FDTD_2D_iterate_Hz(fdtd);
    ++n;
    return n;
}

long iterate_section_4(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    if (0 == n) {
        copy_simulation_up_to(fdtd, 2*N_P);
    } else {
        FDTD_2D_iterate_Hz(fdtd);
        FDTD_2D_iterate_Ex(fdtd);
        FDTD_2D_iterate_Ey(fdtd);
    }
    ++n;
    return n;
}

long iterate_section_5(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    long S = (long)round(6 * N_P * fdtd->gamma_x);
    if (0 == n) {
        copy_simulation_up_to(fdtd, 2*N_P);
    } else {
        FDTD_2D_iterate_Hz(fdtd);
        FDTD_2D_iterate_Ex(fdtd);
        FDTD_2D_iterate_Ey(fdtd);
        place_PEC_obstacle(fdtd, S, J/2, J/2);
    }
    ++n;
    return n;
}

long iterate_section_6(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    /* Excitation on the surface i = 0 */
    for (j = 0; j < J; ++j) {
        E_Y(0, j) = fdtd_excitation2(n, N_P) * 377.0;
    }
    /* Regular solution for 0<i<I */
    for (i = 1; i < I; ++i) {
        for (j = 0; j < J; ++j) {
            E_Y(i, j) += -alpha_Ey * (H_Z(i, j) - H_Z(i-1, j));
        }
    }
    /* PEC at the surface i = I */
    for (j = 0; j < J; ++j) {
        E_Y(I, j) = 0;
    }
    /* Ex and Hz are calculated regularly */
    FDTD_2D_iterate_Ex(fdtd);
    FDTD_2D_iterate_Hz(fdtd);
    ++n;
    return n;
}

long iterate_section_7(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    if (0 == n) {
        copy_simulation_up_to(fdtd, 2*N_P);
    } else {
        FDTD_2D_iterate_Hz(fdtd);
        FDTD_2D_iterate_Ex(fdtd);
        FDTD_2D_iterate_Ey_mur(fdtd);
    }
    ++n;
    return n;
}

long iterate_section_8(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    long S = (long)round(6 * N_P * fdtd->gamma_x);
    if (0 == n) {
        copy_simulation_up_to(fdtd, 2*N_P);
    } else {
        FDTD_2D_iterate_Hz(fdtd);
        FDTD_2D_iterate_Ex(fdtd);
        FDTD_2D_iterate_Ey_mur(fdtd);
        place_PEC_obstacle(fdtd, S, J/2, J/2);
    }
    ++n;
    return n;
}

long iterate_section_9(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    long i = 0, j = 0;
    long S = (long)round(6 * N_P * fdtd->gamma_x);
    if (0 == n) {
        copy_simulation_up_to(fdtd, 2*N_P);
    } else {
        FDTD_2D_iterate_Hz_PML(fdtd, 20, 1);
        FDTD_2D_iterate_Ex_PML(fdtd, 20, 1);
        FDTD_2D_iterate_Ey_PML(fdtd, 20, 1);
        place_PEC_obstacle(fdtd, S, J/2, J/2);
    }
    ++n;
    return n;
}

/* Render functions */

void render_regular(cairo_t *c, fdtd_2d_t *fdtd)
{
    FDTD_2D_render_Hz(c, fdtd);
}

void render_with_obstacle(cairo_t *c, fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    long S = (long)round(6 * N_P * fdtd->gamma_x);
    FDTD_2D_render_Hz(c, fdtd);
    cairo_set_source_rgb(c, 0.0f, 0.0f, 0.0f);
    cairo_rectangle(c, S-J/4-0.5f, J/4+0.5f, J/2, J/2);
    cairo_fill(c);
}

/* Helper functions */

void copy_simulation_up_to(fdtd_2d_t *fdtd, long T)
{
    FDTD_2D_BEGIN(fdtd);
    fdtd_2d_t tmp_fdtd;
    long m = 0;
    long i = 0, j = 0;
    /* Allow the simulation in section 3 for gamma=1 to run until 2*N_P */
    FDTD_2D_init(&tmp_fdtd, 1.0f);
    while (m < T) {
        m = iterate_section_3(&tmp_fdtd);
    }
    /* Now copy Hz of the temporary simulation to the active simulation.
     * We need to copy 2*N_P*gamma=2*N_P cells in x direction */
    for (i = 0; i < T; ++i) {
        for (j = 0; j < J; ++j) {
            H_Z(i, j) = tmp_fdtd.Hz[i];
            E_X(i, j) = tmp_fdtd.Ex[i];
            E_Y(i, j) = tmp_fdtd.Ey[i];
        }
    }
    /* we can free the memory the temporary simulation occupied */
    FDTD_2D_free(&tmp_fdtd);
}

void place_PEC_obstacle(fdtd_2d_t *fdtd, long center_x, long center_y, long side)
{
    long i = 0, j = 0;
    FDTD_2D_BEGIN(fdtd);
    for (i = (center_x - side/2); i < (center_x + side/2); ++i) {
        E_X(i, center_y - side/2) = 0;
        E_X(i, center_y + side/2) = 0;
    }
    for (j = (center_y - side/2); j < (center_y + side/2); ++j) {
        E_Y(center_x - side/2, j) = 0;
        E_Y(center_x + side/2, j) = 0;
    }
}
