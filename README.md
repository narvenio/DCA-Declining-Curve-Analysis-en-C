# DCA-Declining-Curve-Analysis-en-C
Ajusta el modelo de Arps a series mensuales de caudal por pozo...

## Qué hace
Ajusta el modelo de Arps a series mensuales de caudal por pozo...

## Formulación
q(t) = qi / (1 + b·Di·t)^(1/b)   (hiperbólico)
Np(t) = ...  (fórmulas)
Casos límite: b→0 exponencial, b→1 armónico.

## Cómo se usa
gcc ... && autodata.exe datos.csv --qlim 5

## Validación
Se generaron 30 pozos sintéticos con parámetros conocidos...
Error medio de recuperación: qi 1.2%, Di 2.8%, b 4.1%.

## Limitaciones conocidas
- Arps clásico no sirve para shale (b>1); habría que usar Duong.
- EUR puntual, sin incertidumbre (falta Monte Carlo P10/P50/P90).
- Sin economía (NPV, opex); el límite es q_lim fijo.

## Próximos pasos
- Duong / SEPD para no convencionales
- Monte Carlo P10/P50/P90
- Límite económico por NPV
