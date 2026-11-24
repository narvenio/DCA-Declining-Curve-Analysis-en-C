#include "arps.h"
#include <math.h>

double arps_q(const ArpsFit *f, double t)
{
    if (f->b < B_EPS)
        return f->qi * exp(-f->di * t);
    return f->qi * pow(1.0 + f->b * f->di * t, -1.0 / f->b);
}

double arps_np(const ArpsFit *f, double t)
{
    double q = arps_q(f, t);
    if (f->b < B_EPS)
        return (f->qi - q) / f->di;
    if (fabs(f->b - 1.0) < B_EPS)
        return f->qi / f->di * log(f->qi / q);
    return pow(f->qi, f->b) / (f->di * (1.0 - f->b))
            * (pow(f->qi, 1.0 - f->b) - pow(q, 1.0 - f->b));
}

double arps_time_to_q(const ArpsFit *f, double q_lim)
{
    if (q_lim >= f->qi) return 0.0;
    if (f->b < B_EPS)
        return log(f->qi / q_lim) / f->di;
    return (pow(f->qi / q_lim, f->b) - 1.0) / (f->b * f->di);
}