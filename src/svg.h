#ifndef SVG_H
#define SVG_H

#include "types.h"

/* Grafica semilogaritmica: datos reales + curva Arps + limite economico.
    t_end = meses de pronostico contados desde el pico (t0).
*/
int svg_plot_well(const char *patch, const Well *w, const ArpsFit *f, double t0, double t_end, double q_lim);
#endif