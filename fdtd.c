#include "fdtd.h"

#include <math.h>
#include <stdlib.h>

void FDTD_init(fdtd_t *fdtd, double width, double dx, double T, double dt)
{
    fdtd->dx = dx;
    fdtd->I = (long)floor(width / dx);
    /* Correct the width to be a multiple of dx */
    fdtd->width = dx * fdtd->I;
    fdtd->dt = dt;
    fdtd->N = (long)floor(T / dt);
    /* Correct T */
    fdtd->T = dt * fdtd->N;
    fdtd->gamma = dt / dx;
    fdtd->E = (double *)malloc(sizeof(double) * fdtd->I);
    fdtd->H = (double *)malloc(sizeof(double) * (fdtd->I - 1));

    fdtd->N_P = fdtd->I / 10;
}

long FDTD_iterate(fdtd_t *fdtd)
{
    static long n = 0;
    long i = 0;
    if (0 == n) {
        /* First iteration */
        fdtd->E[0] = fdtd_excitation(0, fdtd->N_P);
        for (i = 1; i < fdtd->I; ++i) {
            fdtd->E[i] = 0;
        }
        /* H is anti-symmetric in time */
        for (i = 0; i < (fdtd->I) - 1; ++i) {
            fdtd->H[i] = -0.5 * (fdtd->E[i + 1] - fdtd->E[i]) * fdtd->gamma / MU0;
        }
    } else {
        fdtd->E[0] = fdtd_excitation(n, fdtd->N_P);
        /* We loop until I-1 because there's a wall there and it requires
           different treatment */
        for (i = 1; i < (fdtd->I) - 1; ++i) {
            fdtd->E[i] = fdtd->E[i] - (fdtd->H[i] - fdtd->H[i - 1]) * fdtd->gamma / EPS0;
        }
        /* The waveguide is terminated with a shortcircuit */ 
        fdtd->E[(fdtd->I) - 1] = 0;
        /* Here we loop until I-1 because the H vector is just shorter by 1 */
        for (i = 0; i < (fdtd->I) - 1; ++i) {
            fdtd->H[i] = fdtd->H[i] - (fdtd->E[i + 1] - fdtd->E[i]) * fdtd->gamma / MU0;
        }
    }
    ++n;
    return n;
}

double fdtd_excitation(long n, long N_P)
{
    if ((n >=0) && (n <= 2*N_P)) {
        return pow(sin(M_PI * (n - N_P) / N_P), 3.0f);
    } else {
        return 0;
    }
}
