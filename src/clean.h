#ifndef CLEAN_H
#define CLEAN_H

#include "types.h"

#define MIN_POINTS 6

/* Copia a t_out/q_out el tramo de declinacion (desde el pico, sin ceros).
    t_out se queda relativo al pico; *t0 = tiempo absoluto del pico.
    Devuelve el numero de puntos (O si no hay sufientes).*/
size_t extract_decline(const Well *w, double *t_out, double *q_out, double *t0);

#endif
