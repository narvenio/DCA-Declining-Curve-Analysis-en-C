#ifndef FIT_H
#define FIT_H

#include "types.h"

// Ajusta Arps a (t,q). lq_scratch: buffer de n doubles (evita malloc por pozo)
ArpsFit fit_arps(const double *t, const double *q, size_t n, double *lq_scartch);

#endif