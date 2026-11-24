#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int well_push(Well *w, double t, double q)
{
    if (w->n == w->cap){
        size_t ncap = w->cap ? w->cap * 2 : 64;
        double *nt = realloc(w->t, ncap * sizeof *nt);
        if (!nt) return -1;
        w->t = nt;
        double *nq = realloc(w->q, ncap * sizeof *nq);
        if (!nq) return -1;
        w->q = nq;
        w->cap = ncap;
    }
    w->t[w->n] = t;
    w->q[w->n] = q;
    w->n++;
    return 0;
}

static Well *set_add(WellSet *ws, const char *name)
{
    if (ws->count == ws->cap){
        size_t ncap = ws->cap ? ws->cap * 2 : 16;
        Well *ni = realloc(ws->items, ncap * sizeof *ni);
        if (!ni) return NULL;
        ws->items = ni;
        ws->cap = ncap;
    }
    Well *w = ws->items + ws->count++;
    memset(w, 0, sizeof *w);
    snprintf(w->name, sizeof w->name, "%.*s", WELL_NAME_LEN - 1, name);
    return w;
}

int wellset_load_csv(const char *patch, WellSet *ws)
{
    memset(ws, 0, sizeof *ws);
    FILE *fp = fopen(patch, "r");
    if (!fp) return -1;

    char line[512];
    while (fgets(line, sizeof line, fp)) {
        char *c1 = strchr(line, ',');
        if (!c1) continue;
        char *c2 = strchr(c1 + 1, ',');
        if (!c2) continue;
        *c1 = *c2 = '\0'; // corta la liunea en 3 campos

        char *end; 
        double t = strtod(c1 + 1, &end);
        if (end == c1 + 1) continue; // encabezado o dato invalido
        double q = strtod(c2 + 1, &end);
        if (end == c2 + 1) continue;

        // se recalcula el puntero cada vez; set_add puede hacer realloc
        Well *w = ws->count ? ws->items + ws->count - 1 : NULL;
        if (!w || strcmp(w->name, line) != 0){
            w = set_add(ws, line);
            if (!w) { fclose(fp); return -1;}
        }
        if (well_push(w, t,q) != 0) { fclose(fp); return -1; }
    }
    fclose(fp);
    return 0;
}

void wellset_free(WellSet *ws)
{
    for (Well *w = ws->items, *e = ws->items + ws->count; w < e; w++){
        free(w->t);
        free(w->q);
    }
    free(ws->items);
    memset(ws, 0, sizeof *ws);
}