#include "excitation.h"
#include "fdtd2d.h"

#include <stdio.h>
#include <cairo/cairo.h>
#include <math.h>
#include <stdlib.h>

void FDTD_2D_init(fdtd_2d_t *fdtd, double gamma)
{
    long i = 0;
    fdtd->N_P = 100;
    fdtd->I = (long)ceil(16 * fdtd->N_P * gamma);
    /*fdtd->J = (long)ceil(1.6 * fdtd->N_P * gamma / 3);*/
    fdtd->J = (long)ceil(1.6 * fdtd->N_P * gamma);
    printf("IxJ = %ldx%ld\nN_P = %ld\n", fdtd->I, fdtd->J, fdtd->N_P);
    fdtd->gamma_x = gamma;
    fdtd->gamma_y = gamma;
    fdtd->Hz = (double *)malloc(sizeof(double) * (fdtd->I) * (fdtd->J));
    fdtd->Ey = (double *)malloc(sizeof(double) * (fdtd->I + 1) * (fdtd->J));
    fdtd->Ex = (double *)malloc(sizeof(double) * (fdtd->I) * (fdtd->J + 1));
    for (i = 0; i < (fdtd->I) * (fdtd->J); ++i) fdtd->Hz[i] = 0.0f;
    for (i = 0; i < (fdtd->I + 1) * (fdtd->J); ++i) fdtd->Ey[i] = 0.0f;
    for (i = 0; i < (fdtd->I) * (fdtd->J + 1); ++i) fdtd->Ex[i] = 0.0f;
}

void FDTD_2D_free(fdtd_2d_t *fdtd)
{
    free(fdtd->Hz);
    free(fdtd->Ex);
    free(fdtd->Ey);
}

void FDTD_2D_iterate_Hz(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    long i = 0, j = 0;
    for (i = 0; i < I; ++i) {
        for (j = 0; j < J; ++j) {
            H_Z(i, j) += -alpha_Hz_Ey * (E_Y(i+1, j) - E_Y(i, j)) +
                          alpha_Hz_Ex * (E_X(i, j+1) - E_X(i, j));
        }
    }
}

void FDTD_2D_iterate_Ex(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    long i = 0, j = 0;
    for (i = 0; i < I; ++i) {
        for (j = 1; j < J; ++j) {
            E_X(i, j) += alpha_Ex * (H_Z(i, j) - H_Z(i, j-1));
        }
    }
}

void FDTD_2D_iterate_Ey(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    long i = 0, j = 0;
    for (i = 1; i < I; ++i) {
        for (j = 0; j < J; ++j) {
            E_Y(i, j) += -alpha_Ey * (H_Z(i, j) - H_Z(i-1, j));
        }
    }
}

/* Mur boundary condition */

void FDTD_2D_iterate_Ey_mur(fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    static double *prev_Ey = NULL;
    static double *prev_prev_Ey = NULL;
    static long n = 1;
    long i = 0, j = 0;
#define P_E_Y(x,y) prev_Ey[((x)-I+1+10)+(y)*2]
#define PP_E_Y(x,y) prev_prev_Ey[((x)-I+1+10)+(y)*2]
    if (1 == n) {
        prev_Ey = (double *)malloc(sizeof(double) * 2 * J);
        prev_prev_Ey = (double *)malloc(sizeof(double) * 2 * J);
        for (i = 0; i < 2*J; ++i) {
            prev_Ey[i] = 0.0f;
            prev_prev_Ey[i] = 0.0f;
        }
    }
    FDTD_2D_iterate_Ey(fdtd);
    /* One-way wave equation for the plane i = I */
    for (j = 0; j < J; ++j) {
        E_Y(I-10, j) = -PP_E_Y(I-1-10, j) +
                       C1*(E_Y(I-1-10, j) + PP_E_Y(I-10, j)) +
                       C2*(P_E_Y(I-1-10, j) + P_E_Y(I-10, j));
    }
    /* Copy the fields at (n-1) to prevprev, and the fields at (n) to prev */
    for (i = (I-1-10); i < (I+1-10); ++i) {
        for (j = 0; j < J; ++j) {
            PP_E_Y(i, j) = P_E_Y(i, j);
            P_E_Y(i,j) = E_Y(i, j);
        }
    }
    ++n;
}

/* PML boundary condition */
void FDTD_2D_iterate_Ex_PML(fdtd_2d_t *fdtd, long L, double alpha_max)
{
    FDTD_2D_iterate_Ex(fdtd);
}

void FDTD_2D_iterate_Ey_PML(fdtd_2d_t *fdtd, long L, double alpha_max)
{
    FDTD_2D_BEGIN(fdtd);
    long i = 0, j = 0;
    double alpha = 0;
    double coeff1 = 0, coeff2 = 0;
    /* Regular LeapFrog */
    for (i = 1; i < (I-L); ++i) {
        for (j = 0; j < J; ++j) {
            E_Y(i, j) += -alpha_Ey * (H_Z(i, j) - H_Z(i-1, j));
        }
    }
    /* PML equations with a parabolic conductivity profile */
    for (i = (I-L); i < I; ++i) {
        alpha = alpha_max * pow(((double)(i-I+L))/((double)L), 2.0f);
        coeff1 = (1.0f - alpha)/(1.0f + alpha);
        coeff2 = fdtd->gamma_x / (EPS0 * C0 * (1.0 + alpha));
        for (j = 0; j < J; ++j) {
            E_Y(i, j) = coeff1 * E_Y(i, j) -
                        coeff2 * (H_Z(i, j) - H_Z(i-1, j));
        }
    }
}

void FDTD_2D_iterate_Hz_PML(fdtd_2d_t *fdtd, long L, double alpha_max)
{
    FDTD_2D_BEGIN(fdtd);
    static long n = 0;
    static double *Hzx = NULL, *Hzy = NULL;
#define H_ZX(x,y) Hzx[((x)-I+L)+(y)*L]
#define H_ZY(x,y) Hzy[((x)-I+L)+(y)*L]
    long i = 0, j = 0;
    double alpha = 0.0f;
    double coeff1 = 0.0f, coeff2 = 0.0f, coeff3 = 0.0f;
    if (0 == n) {
        /* Hz is comprised of Hzx and Hzy. Both Hzx and Hzy are not "real"
         * Field values, but we need them for the computation, therefore they
         * will be internal to this function, and not pollute the fdtd_2d_t
         * structure */
        Hzx = (double *)malloc(sizeof(double) * L * J);
        Hzy = (double *)malloc(sizeof(double) * L * J);
        for (i = 0; i < (L * J); ++i) {
            Hzx[i] = 0.0f;
            Hzy[i] = 0.0f;
        }
    } else {
        /* Regular LeapFrog */
        for (i = 0; i < (I-L); ++i) {
            for (j = 0; j < J; ++j) {
                H_Z(i, j) += -alpha_Hz_Ey * (E_Y(i+1, j) - E_Y(i, j)) +
                              alpha_Hz_Ex * (E_X(i, j+1) - E_X(i, j));
            }
        }
        /* PML equations with a parabolic conductivity profile */
        for (i = (I-L); i < I; ++i) {
            alpha = alpha_max * pow(((double)(i-I+L))/((double)L), 2.0f);
            coeff1 = (1.0f - alpha)/(1.0f + alpha);
            coeff2 = fdtd->gamma_x / (MU0 * C0 * (1.0 + alpha));
            coeff3 = fdtd->gamma_y / (MU0 * C0);
            for (j = 0; j < J; ++j) {
                H_ZX(i, j) = coeff1 * H_ZX(i, j) -
                             coeff2 * (E_Y(i+1, j) - E_Y(i, j));
                H_ZY(i, j) = H_ZY(i, j) +
                             coeff3 * (E_X(i, j+1) - E_X(i, j));
                H_Z(i, j) = H_ZX(i, j) + H_ZY(i, j);
            }
        }
    }
    ++n;
}

/* Render function */
void FDTD_2D_render_Hz(cairo_t *c, fdtd_2d_t *fdtd)
{
    FDTD_2D_BEGIN(fdtd);
    long i = 0;
    long j = 0;
    double x, y;
    double mag = 0;
#if 0
    double max_Hz = -1;
    for (i = 0; i < (I*J); ++i) {
        if (Hz[i] < 0) {
            if (-Hz[i] > max_Hz) max_Hz = -Hz[i];
        } else {
            if (Hz[i] > max_Hz) max_Hz = Hz[i];
        }
    }
#else
    double max_Hz = 1.0f;
#endif
    for (i = 0; i < I; ++i) {
        x = i + 1;
        for (j = 0; j < J; ++j) {
            y = J - j;
            mag = round(H_Z(i, j)/max_Hz * 64) / 64;
            if (mag == 0) continue;
            if (mag > 0) {
                cairo_set_source_rgb(c, 1.0f-mag, 1.0f-mag, 1.0f);
                cairo_rectangle(c, x-0.5f, y-0.5f, 1, 1);
                cairo_fill(c);
            } else {
                cairo_set_source_rgb(c, 1.0, 1.0f+mag, 1.0f+mag);
                cairo_rectangle(c, x-0.5f, y-0.5f, 1, 1);
                cairo_fill(c);
            }
        }
    }
}
