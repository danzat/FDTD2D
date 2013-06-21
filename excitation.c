#include "excitation.h"
#include <math.h>

double fdtd_excitation(long n, long N_P)
{
    if ((n >=0) && (n <= 2*N_P)) {
        return pow(sin(M_PI * (n - N_P) / N_P), 3.0f);
    } else {
        return 0;
    }
}

double fdtd_excitation2(long n, long N_P)
{
    if ((n >=0) && (n <= 6)) {
        return 1.0f;
    } else {
        return 0;
    }
}

