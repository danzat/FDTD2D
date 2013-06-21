#ifndef FDTD_2D_H
#define FDTD_2D_H

#include <math.h>
#include <cairo.h>

#define C0      (299792458.0f)
#define MU0     (M_PI*4e-7)
#define EPS0    (8.8542e-12)

typedef struct {
    long    I;      /* Discrete simulation horizontal(x) space */
    long    J;      /* Discrete simulation vertical(y) space */
    double  *Hz;    /* Magnetic field z magnitude */
    double  *Ex;    /* Electric field x magnitude */
    double  *Ey;    /* Electric field y magnitude */
    double  gamma_x;  /* dt / dx */
    double  gamma_y;  /* dt / dy */
    /* Excitation specific data */
    long N_P;
} fdtd_2d_t;

#define FDTD_2D_BEGIN(fdtd) \
    long I = (fdtd)->I;\
    long J = (fdtd)->J;\
    long N_P = (fdtd)->N_P;\
    double *Ex = (fdtd)->Ex;\
    double *Ey = (fdtd)->Ey;\
    double *Hz = (fdtd)->Hz;\
    double alpha_Hz_Ey = (fdtd)->gamma_x / (C0 * MU0);\
    double alpha_Hz_Ex = (fdtd)->gamma_y / (C0 * MU0);\
    double alpha_Ex = (fdtd)->gamma_y / (C0 * EPS0);\
    double alpha_Ey = (fdtd)->gamma_x / (C0 * EPS0)

#define H_Z(x,y) Hz[(x)+(y)*I]
#define E_X(x,y) Ex[(x)+(y)*I]
#define E_Y(x,y) Ey[(x)+(y)*(I+1)]

void FDTD_2D_init(fdtd_2d_t *fdtd, double gamma);
void FDTD_2D_free(fdtd_2d_t *fdtd);

void FDTD_2D_iterate_Hz(fdtd_2d_t *fdtd);
void FDTD_2D_iterate_Ex(fdtd_2d_t *fdtd);
void FDTD_2D_iterate_Ey(fdtd_2d_t *fdtd);

#define C1 ((fdtd->gamma_x - 1.0f)/(fdtd->gamma_x + 1.0f))
#define C2 (2.0f/(fdtd->gamma_x + 1.0f))
void FDTD_2D_iterate_Hz_mur(fdtd_2d_t *fdtd);
void FDTD_2D_iterate_Ex_mur(fdtd_2d_t *fdtd);
void FDTD_2D_iterate_Ey_mur(fdtd_2d_t *fdtd);

void FDTD_2D_iterate_Hz_PML(fdtd_2d_t *fdtd, long L, double alpha_max);
void FDTD_2D_iterate_Ex_PML(fdtd_2d_t *fdtd, long L, double alpha_max);
void FDTD_2D_iterate_Ey_PML(fdtd_2d_t *fdtd, long L, double alpha_max);

void FDTD_2D_render_Hz(cairo_t *c, fdtd_2d_t *fdtd);
void FDTD_2D_render_ExEy(cairo_t *c, fdtd_2d_t *fdtd);

#endif /* FDTD_2D_H */
