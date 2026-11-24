#!/bin/bash
# validar.sh — compara parámetros reales vs ajustados
paste -d',' <(grep 'POZO' pozos_parametros.txt | sed 's/.*qi=\([0-9.]*\) Di=\([0-9.]*\) b=\([0-9.]*\)/\1,\2,\3/') \
             <(tail -n +2 resultados/resumen.csv | cut -d',' -f2-4) \
| awk -F',' 'BEGIN{print "qi_real,Di_real,b_real,qi_aj,Di_aj,b_aj,err_qi%,err_Di%,err_b%"}
{
  eq = ($1>0)? 100*($4-$1)/$1 : 0
  ed = ($2>0)? 100*($5-$2)/$2 : 0
  eb = ($3>0)? 100*($6-$3)/$3 : 0
  printf "%s,%s,%s,%s,%s,%s,%.1f,%.1f,%.1f\n",$1,$2,$3,$4,$5,$6,eq,ed,eb
}' > resultados/validacion.csv