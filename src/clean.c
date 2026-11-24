#include "clean.h"

/* Pico sobre una media movil de 3 puntos (resiste valores atipicos aislados)*/
static size_t find_peak(const double *q, size_t n)
{
    size_t best = 0;
    double bv = -1.0;
    for (size_t i = 0; i < n; i++){
        const double *a = q + (i ? i - 1 : i);
        const double *b = q + (i + 1 < n ? i + 1 : i);
        double s = (*a + q[i] + *b) / 3.0;
        if (s > bv) { bv = s; best = i; }
    }
    return best;
}

size_t extract_decline(const Well *w, double *t_out, double *q_out, double *t0)
{
    if (w->n < MIN_POINTS) return 0;

    size_t pk = find_peak(w->q, w->n);
    *t0 = w->t[pk];

    const double *tp  = w->t + pk;
    const double *qp  = w->q + pk;
    const double *end = w->t + w->n;
    size_t m = 0;

    for(; tp < end; tp++, qp++){
        if (*qp > 0.0){
            t_out[m] = *tp - *t0;
            q_out[m] = *qp;
            m++;
        }
    }
    return m;
}