#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir((p), 0755)
#endif

#include "arps.h"
#include "clean.h"
#include "csv.h"
#include "fit.h"
#include "svg.h"
#include "types.h"

#define DAYS_PER_MONTH 30.4375

static void usage(const char *prog){
     fprintf(stderr,
        "Uso: %s datos.csv [--qlim bbl/d] [--tmax meses] [--out carpeta]\n"
        "  CSV: pozo,mes,caudal  (filas agrupadas por pozo)\n", prog);
}

static void safe_name(char *dst, size_t cap, const char *src){
    size_t i = 0;
    for (; *src && i + 1 < cap; src++, i++)
        dst[i] = isalnum((unsigned char)*src) ? *src : '_';
    dst[i] = '\0';
}

int main(int argc, char **argv){
    if (argc < 2){ usage(argv[0]); return 1; }

    const char *in = argv[1], *outdir = "resultados";
    double qlim = 5.0, tmax = 600.0;
    for (int i = 2; i < argc; i++){
        if      (!strcmp(argv[i], "--qlim") && i + 1 < argc) qlim = atof(argv[++i]);
        else if (!strcmp(argv[i], "--tmax") && i + 1 < argc) tmax = atof(argv[++i]);
        else if (!strcmp(argv[i], "--out")  && i + 1 < argc) outdir = argv[++i];
        else { usage(argv[0]); return 1; }
    }
    if (qlim <= 0.0 || tmax <= 0.0) { usage(argv[0]); return 1; }

    WellSet ws;
    if (wellset_load_csv(in, &ws) != 0 || ws.count == 0){
        fprintf(stderr, "No se pudo leer datos validos de %s\n", in);
        wellset_free(&ws);
        return 1;
    }

    /* Un solo malloc para los 3 buffers de trabajo, reutilizados en todos los pozos */
    size_t maxn = 0;
    for (const Well *w = ws.items, *e = ws.items + ws.count; w < e; w++)
        if (w->n > maxn) maxn = w->n;
    
    double *buf = malloc(3 * maxn * sizeof *buf);
    if (!buf) { wellset_free(&ws); return 1; }
    double *tb = buf, *qb = buf + maxn, *lq = buf + 2 * maxn;

    MKDIR(outdir);
    char path[512];
    snprintf(path, sizeof path, "%s/resumen.csv", outdir);
    FILE *sum = fopen(path, "w");
    if (sum) fprintf(sum, "pozo, qi, Di,b,R2,RMSE,EUR_bbl,t_abandono_meses\n");
    printf("%-12s %8s %8s %5s %6s %13s %9s\n",
           "Pozo", "qi", "Di", "b", "R2", "EUR(bbl)", "t_ab(m)");
    double total = 0.0;
    size_t okc = 0;

    for (Well *w = ws.items, *e = ws.items + ws.count; w < e; w++){
        double t0;
        size_t n = extract_decline(w, tb, qb, &t0);
        ArpsFit f = fit_arps(tb, qb, n, lq);
        if (!f.ok) {
            printf("%-12s (datos insuficientes o sin declinacion)\n", w->name);
            continue;
        }

        double tend = arps_time_to_q(&f, qlim);
        if (tend > tmax) tend = tmax;
        double tlast = tb[n - 1];

        double cum = 0.0; /* producido real*/
        for (const double *p = w->q, *pe = w->q + w->n; p < pe; p++) cum += *p;
        cum *= DAYS_PER_MONTH;

        double rem = (tend > tlast) /* pronostico restante*/
            ? (arps_np(&f, tend) - arps_np(&f, tlast)) * DAYS_PER_MONTH : 0.0;
        double eur = cum + rem;

        printf("%-12s %8.1f %8.4f %5.2f %6.3f %13.0f %9.0f\n",
               w->name, f.qi, f.di, f.b, f.r2, eur, tend);
        if (sum)
            fprintf(sum, "%s,%.2f,%.5f,%.3f,%.4f,%.2f,%.0f,%.1f\n",
                    w->name, f.qi, f.di, f.b, f.r2, f.rmse, eur, tend);
                
        char nm[WELL_NAME_LEN];
        safe_name(nm, sizeof nm, w->name);
        snprintf(path, sizeof path, "%s/%s.svg", outdir, nm);
        svg_plot_well(path, w, &f, t0, tend, qlim);

        total += eur;
        okc++;
    }

    printf("\nPozos ajustados: %zu / %zu   EUR de campo: %.0f bbl\n",
           okc, ws.count, total);
    
    if (sum) fclose(sum);
    free(buf);
    wellset_free(&ws);
    return 0;
}