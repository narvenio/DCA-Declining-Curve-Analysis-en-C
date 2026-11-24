#ifndef ARPS_H
#define ARPS_H

#include "types.h"

#define B_EPS 1e-6 // b < B_EPS -> exponencial; |b-1| < B_EPS -> armonica

double arps_q(const ArpsFit *f, double t); // caudal en t
double arps_np(const ArpsFit *f, double t); // acumulado 0..t
double arps_time_to_q(const ArpsFit *f, double q_lim); // t hasta q_lim

#endif