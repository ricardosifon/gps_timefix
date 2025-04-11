# gps_timefix.d - Configuración para el módulo Earthworm gps_timefix
#
# Este módulo toma datos de tipo TYPE_TRACEBUF2 con problemas de tiempo (por GPS roto)
# y los corrige, reemplazando starttime y endtime por tiempos locales del sistema.
# Los resultados se escriben en un anillo de salida.

# === Nombre del módulo (como está definido en earthworm.d) ===
MyModuleId        MOD_GPS_TIMEFIX

# === Nombre del anillo de memoria compartida de entrada ===
RingName          GPS_IN_RING

# === Nombre del anillo de memoria compartida de salida ===
OutRingName       GPS_OUT_RING

# === Intervalo del heartbeat (segundos) ===
HeartBeatInterval   30

# === Nivel de log (0 = nada, 1 = salida por logit) ===
LogFile             2

# === Qué mensajes leer ===
# Instancia, módulo y tipo de mensaje que queremos procesar
GetEventsFrom       INST_WILDCARD MOD_WILDCARD TYPE_TRACEBUF2
