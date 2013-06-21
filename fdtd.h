#ifndef FDTD_H
#define FDTD_H

#include <math.h>

#define C0      (299792458.0f)
#define MU0     (M_PI*4e-7)
#define EPS0    (8.8542e-12)

typedef struct {
    double  width;  /* Width of the computation space */
    double  dx;     /* Spatial resolution */ 
    double  T;      /* Total simulation time */
    double  dt;     /* Temporal resolution */
    long    I;      /* Discrete simulation space */
    long    N;      /* Discrete simulation time */
    double  *E;    /* Electrical field magnitude */
    double  *H;    /* Magnetic field magnitude */
    double  gamma;  /* dt / dx */
    /* Excitation specific data */
    long N_P;
} fdtd_t;

void FDTD_init(fdtd_t *fdtd, double width, double dx, double T, double dt);
long FDTD_iterate(fdtd_t *fdtd);

double fdtd_excitation(long n, long N_P);
#endif /* FDTD_H */
