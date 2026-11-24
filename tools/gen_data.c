#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static double urand(void) { return rand() / (double)RAND_MAX; }

int main(int argc, char **argv)
{
    int wells = (argc > 1) ? atoi(argv[1]) : 20;
    srand(12345);
    puts("well,month,rate");

    for (int k = 1; k <= wells; k++) {
        double qi = 300.0 + 1200.0 * urand();
        double di = 0.02 + 0.10 * urand();
        double b  = 1.2 * urand();
        int months = 36 + rand() % 37;
        fprintf(stderr, "POZO-%03d qi=%.1f Di=%.4f b=%.2f\n", k, qi, di, b);

        for (int m = 0; m < months; m++) {
            double q;
            if (m < 2) {
                q = qi * (0.6 + 0.2 * m);                 /* arranque */
            } else {
                double tau = m - 2.0;
                q = (b < 1e-6) ? qi * exp(-di * tau)
                               : qi * pow(1.0 + b * di * tau, -1.0 / b);
            }
            q *= 1.0 + 0.05 * (2.0 * urand() - 1.0);      /* ruido +-5 % */
            printf("POZO-%03d,%d,%.2f\n", k, m, q);
        }
    }
    return 0;
}