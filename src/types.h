#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#define WELL_NAME_LEN 64

typedef struct
{
    char name[WELL_NAME_LEN];
    size_t n, cap;
    double *t; // Tiempo en meses
    double *q; // caudal (bbl/d)
} Well;

typedef struct 
{
    Well *items;
    size_t count, cap;
} WellSet;

typedef struct 
{
    double qi, di, b; // di en 1/mes
    double r2, rmse;
    int ok;    
} ArpsFit;

#endif


