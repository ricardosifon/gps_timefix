# gps_timefix — Módulo Earthworm para corrección de timestamps

`gps_timefix` es un módulo de Earthworm que corrige mensajes `TYPE_TRACEBUF2` con problemas de fecha y hora (por ejemplo, cuando falló el GPS en una estación sísmica). Reemplaza los campos `starttime` y `endtime` por una secuencia continua de tiempo local del sistema.

## 🧩 Funcionalidad

- Lee mensajes `TYPE_TRACEBUF2` de un anillo de entrada.
- Detecta la estación y canal (STA.CHAN.NET.LOC).
- Corrige los campos `starttime` y `endtime` de cada traza.
- Mantiene continuidad temporal sin gaps ni overlaps.
- Escribe los mensajes corregidos en un anillo de salida.

## 📦 Instalación

1. Clonar o copiar el archivo `gps_timefix.c` en:

2. Copiar el `makefile.unix` provisto en el mismo directorio.

3. Compilar con:
```bash
make -f makefile.unix
bin/gps_timefix

Licencia y contribuciones
Este módulo está diseñado para ser incorporado a Earthworm y puede ser redistribuido bajo los términos del proyecto. Se agradece cualquier mejora, parche o sugerencia.

Autor: RICARDO GABRIEL SIFON
Fecha: 2025-04-11
