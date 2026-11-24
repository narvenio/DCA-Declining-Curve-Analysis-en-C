#include "fit.h"
#include "arps.h"
#include <math.h>
#include <string.h>

#define NM_N 3 // parametros: ln(qi), ln(Di), b
#define BMAX 1.5
#define PENALTY 1e30

typedef struct
{
    const double *t;
    const double *lq;
    size_t         n;
} FitCtx;

// Puntero a funcion: el optimizador no sabe nada del modelo
typedef double (*ObjFn)(const double *x, const void *ctx);

// Regresion lineal sobre ln(q) = ln(qi) - D t
static int fit_exponential(const double *t, const double *lq, size_t n, double *a, double *d)
{
    double st = 0, sl = 0, stt = 0, stl = 0;
    const double *tp = t, *lp = lq, *end = t + n;
    for (; tp < end; tp++, lp++){
        st  += *tp;
        sl  += *lp;
        stt += *tp * *tp;
        stl += *tp * *lp;
    }
    double dn = (double)n;
    double den = dn * stt - st * st;
    if (fabs(den) < 1e-12) return -1;
    double slope = (dn * stl - st * sl) / den;
    *a = (sl - slope * st) / dn;
    *d = -slope;
    return (*d > 0.0) ? 0 : -1; // si no se inclina, no hay ajuste
}

/* Objetivo: SSE en espacio logaritmico
    ln q = ln qi - (1/b) ln(1 + b D t) -> pow() dentro del bucle
*/
static double obj_hyp(const double *x, const void *vctx)
{
    const FitCtx *c = vctx;
    double d = exp(x[1]);
    double b = x[2];
    if (b < 0.0 || b > BMAX) return PENALTY;

    double sse = 0.0;
    const double *tp = c->t, *lp = c->lq, *end = c->t + c->n;

    if (b < B_EPS) {
        for (; tp < end; tp++, lp++) {
            double r = *lp - (x[0] - d * *tp);
            sse += r * r;
        }
    } else {
        double inv = 1.0 / b;
        for (; tp < end; tp++, lp++){
            double r = *lp - (x[0] - inv * log1p(b * d * *tp));
            sse += r * r;
        }
    }
    return sse;
}

/* Nelder-Mead (simplex en la pila, sin malloc)*/
static void nelder_mead(ObjFn fn, const void *ctx, double *x, const double *step, int max_iter, double tol)
{
    double s[NM_N + 1][NM_N], fv[NM_N + 1];
    double xr[NM_N], xt[NM_N], cen[NM_N];
    int i, j;

    for (i = 0; i <= NM_N; i++){
        memcpy(s[i], x, sizeof s[i]);
        if (i > 0)s[i][i - 1] += step[i -1];
        fv[i] = fn(s[i], ctx);
    }

    for (int it = 0; it < max_iter; it++){
        int lo = 0, hi = 0, nhi;
        for (i = 1; i <= NM_N; i++) {
            if (fv[i] < fv[lo]) lo = i;
            if (fv[i] > fv[hi]) hi = i;
        }
        if (fv[hi] - fv[lo] < tol) break;

        nhi = lo;
        for (i = 0; i <= NM_N; i++)
            if (i != hi && fv[i] > fv[nhi]) nhi = i;
        
        for (j = 0; j < NM_N; j++){ // centroide sin el peor
            cen[j] = 0.0;
            for (i = 0; i <= NM_N; i++)
                if ( i != hi) cen[j] += s[i][j];
            cen[j] /= NM_N;
        }

        for (j = 0; j < NM_N; j++) xr[j] = cen[j] + (cen[j] - s[hi][j]);
        double fr = fn(xr, ctx);

        if (fr < fv[lo]){
            for (j = 0; j < NM_N; j++) xt[j] = cen[j] + 2.0 * (xr[j] - cen[j]);
            double fe = fn(xt, ctx);
            if (fe < fr) { memcpy(s[hi], xt, sizeof xt); fv[hi] = fe; }
            else         { memcpy(s[hi], xr, sizeof xr); fv[hi] = fr; }
        } else if (fr < fv[nhi]){
            memcpy(s[hi], xr, sizeof xr);
            fv[hi] = fr;
        } else { // contracion
            const double *from = (fr < fv[hi]) ? xr : s[hi];
            for (j = 0; j < NM_N; j++) xt[j] = cen[j] + 0.5 * (from[j] - cen[j]);
            double fc   = fn(xt, ctx);
            double ref = (fr < fv[hi]) ? fr : fv[hi];
            if (fc < ref){
                memcpy(s[hi], xt, sizeof xt);
                fv[hi] = fc;
            } else { // encogimiento
                for (i = 0; i <= NM_N; i++){
                    if (i == lo) continue;
                    for (j = 0; j < NM_N; j++)
                        s[i][j] = s[lo][j] + 0.5 * (s[i][j] - s[lo][j]);
                    fv[i] = fn(s[i], ctx);
                }
            }
        }

    }

    int best = 0;
    for (i = 1; i <= NM_N; i++)
        if (fv[i] < fv[best]) best = i;
    memcpy(x, s[best], sizeof s[best]);
}

/* Metricas en espacio lineal*/
static void stats(ArpsFit *f, const double *t, const double *q, size_t n)
{
    double mean = 0.0;
    for (const double *p = q, *e = q + n; p < e; p++) mean += *p;
    mean /= (double)n;

    double sse = 0.0, sst = 0.0;
    const double *tp = t, *qp = q, *end = q + n;
    for (; qp < end; tp++, qp++){
        double r = *qp - arps_q(f, *tp);
        double m = *qp - mean;
        sse += r * r;
        sst += m * m;
    }
    f->r2   = (sst > 0.0) ? 1.0 - sse / sst : 0.0;
    f->rmse = sqrt(sse / (double)n);
}

ArpsFit fit_arps(const double *t, const double *q, size_t n, double *lq)
{
    ArpsFit none = {0};
    if (n < 6) return none;

    for (size_t i = 0; i < n; i++) lq[i] = log(q[i]);

    double a, d;
    if (fit_exponential(t, lq, n, &a, &d) != 0) return none;

    ArpsFit fe = { exp(a), d, 0.0, 0.0, 0.0, 1};
    stats(&fe, t, q, n);

    FitCtx ctx = { t, lq, n };
    double x[NM_N] = { a, log(d), 0.5};
    const double step[NM_N] = { 0.1, 0.5, 0.3};
    for (int r = 0; r < 4; r++)     // reinicios del simplex
        nelder_mead(obj_hyp, &ctx, x, step, 800, 1e-14);

    ArpsFit fh = { exp(x[0]), exp(x[1]), x[2], 0.0, 0.0, 1 };
    stats(&fh, t, q, n);

    // Prefiere el modelo simple salvo mejora clara
    return (fh.b > 0.05 && fh.r2 > fe.r2 + 0.005) ? fh : fe;
}

