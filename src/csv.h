#ifndef CSV_H
#define CSV_H

#include "types.h"

// Lee un CSV "pozo, mes, caudal" (filas agrupadas por pozo). O = OK
int wellset_load_csv(const char *patch, WellSet *ws);
void wellset_free(WellSet *ws);

#endif