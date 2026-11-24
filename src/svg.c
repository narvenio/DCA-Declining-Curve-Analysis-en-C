#include "svg.h"
#include "arps.h"
#include <math.h>
#include <stdio.h>

#define PLOT_W 800.0
#define PLOT_H 500.0
#define M_L     70.0
#define M_R     20.0 
#define M_T     40.0
#define M_B     50.0
#define STEPS   200

static double map_x(double t, double xmax)
{
    return M_L + (t / xmax) * (PLOT_W - M_L - M_R);
}

static double map_y(double q, double lo, double hi)
{
    if (q < 1e-12) q = 1e-12;
    double f = (log10(q) - lo) / (hi - lo);
    return PLOT_H - M_B - f * (PLOT_H - M_T - M_B);
}

int svg_plot_well(const char *path, const Well *w, const ArpsFit *f, double t0, double t_end, double q_lim)
{
    FILE *fp = fopen(path, "w");
    if (!fp) return -1;

    double xmax = t0 + t_end;
    if (w->t[w->n - 1] > xmax) xmax = w->t[w->n - 1];
    if (xmax <= 0.0) xmax = 1.0;

    double qmax = f->qi, qmin = q_lim;
    for (const double *p = w->q, *e = w->q + w->n; p < e; p++){
        if (*p > qmax) qmax = *p;
        if (*p > 0.0 && *p < qmin) qmin = *p;
    }
    double lo = floor(log10(qmin)), hi = ceil(log10(qmax));
    if (hi <= lo) hi = lo + 1.0;

    fprintf(fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%g\" height=\"%g\" "
                "viewBox=\"0 0 %g %g\" font-family=\"sans-serif\" font-size=\"12\">\n",
            PLOT_W, PLOT_H, PLOT_W, PLOT_H);
    fprintf(fp, "<rect width=\"100%%\" height=\"100%%\" fill=\"white\"/>\n");
    fprintf(fp, "<text x=\"%g\" y=\"24\" font-size=\"16\" font-weight=\"bold\">%s</text>\n",
            M_L, w->name);
    fprintf(fp, "<text x=\"%g\" y=\"24\" text-anchor=\"end\">"
                "qi=%.1f  Di=%.4f/mes  b=%.2f  R2=%.3f</text>\n",
            PLOT_W - M_R, f->qi, f->di, f->b, f->r2);
    /* rejilla horizontal: una linea por decada*/
    for (double d = lo; d <= hi; d += 1.0) {
        double y = map_y(pow(10.0, d), lo, hi);
        fprintf(fp, "<line x1=\"%g\" y1=\"%.1f\" x2=\"%g\" y2=\"%.1f\" stroke=\"#ddd\"/>\n",
                M_L, y, PLOT_W - M_R, y);
        fprintf(fp, "<text x=\"%g\" y=\"%.1f\" text-anchor=\"end\">%g</text>\n",
                M_L - 6, y + 4, pow(10.0, d));
    }
    /* Rejilla vertical */
    for (int i = 0; i <= 5; i++){
        double tt = xmax * i / 5.0, x = map_x(tt, xmax);
        fprintf(fp, "<line x1=\"%.1f\" y1=\"%g\" x2=\"%.1f\" y2=\"%g\" stroke=\"#eee\"/>\n",
                x, M_T, x, PLOT_H - M_B);
        fprintf(fp, "<text x=\"%.1f\" y=\"%g\" text-anchor=\"middle\">%.0f</text>\n",
                x, PLOT_H - M_B + 18, tt);
    }
    fprintf(fp, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" fill=\"none\" stroke=\"#333\"/>\n",
            M_L, M_T, PLOT_W - M_L - M_R, PLOT_H - M_T - M_B);
    fprintf(fp, "<text x=\"%g\" y=\"%g\" text-anchor=\"middle\">Tiempo (meses)</text>\n",
            (M_L + PLOT_W - M_R) / 2.0, PLOT_H - 10);
    fprintf(fp, "<text transform=\"translate(18 %g) rotate(-90)\" text-anchor=\"middle\">"
                "Caudal (bbl/d)</text>\n", (M_T + PLOT_H - M_B) / 2.0);

    /* Limite economico*/
    double yl = map_y(q_lim, lo, hi);
    fprintf(fp, "<line x1=\"%g\" y1=\"%.1f\" x2=\"%g\" y2=\"%.1f\" stroke=\"#c0392b\" "
                "stroke-dasharray=\"6 4\"/>\n", M_L, yl, PLOT_W - M_R, yl);
    
    /* datos reales*/
    const double *tp = w->t, *qp = w->q, *end = w->t + w->n;
    for (; tp < end; tp++, qp++){
        if (*qp <= 0.0) continue;
        fprintf(fp, "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"3\" fill=\"#1f77b4\"/>\n",
                map_x(*tp, xmax), map_y(*qp, lo, hi));
    }

    /* curva ajustada + pronostico*/
    fprintf(fp, "<polyline fill=\"none\" stroke=\"#e67e22\" stroke-width=\"2\" points=\"");
    for (int i = 0; i <= STEPS; i++){
        double tau = t_end * i / STEPS;
        fprintf(fp, "%.1f,%.1f ", map_x(t0 + tau, xmax),
                map_y(arps_q(f, tau), lo, hi));
    }
    fprintf(fp, "\"/>\n</svg>\n");
    fclose(fp);
    return 0;
    }
