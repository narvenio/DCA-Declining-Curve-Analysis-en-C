@echo off
setlocal

echo ============================================
echo   Build + Generar + Analizar
echo ============================================
echo.

echo [1/4] Compilando autodata.exe ...
gcc -O2 -Wall -Wextra -std=c11 -o autodata.exe ^
    src\main.c src\arps.c src\clean.c src\csv.c src\fit.c src\svg.c -lm
if errorlevel 1 goto error

echo [2/4] Compilando gen_data.exe ...
gcc -O2 -Wall -Wextra -std=c11 -o gen_data.exe tools\gen_data.c -lm
if errorlevel 1 goto error

echo [3/4] Generando datos sinteticos (30 pozos) ...
gen_data.exe 30 > datos.csv 2> pozos_parametros.txt
if errorlevel 1 goto error

echo [4/4] Analizando ...
autodata.exe datos.csv --qlim 5 --tmax 600 --out resultados
if errorlevel 1 goto error

echo.
echo Listo. Revisa:
echo   - resultados\resumen.csv
echo   - resultados\*.svg
echo   - pozos_parametros.txt (parametros reales usados para generar)
echo.
start "" resultados\resumen.csv
goto end

:error
echo.
echo *** ERROR. Revisa los mensajes de arriba. ***

:end
endlocal
pause