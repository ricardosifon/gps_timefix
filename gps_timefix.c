// gps_timefix.c completo con fake_time por canal basado en hora del sistema actual

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "earthworm.h"
#include "kom.h"
#include "transport.h"
#include "lockfile.h"
#include "trace_buf.h"

#define MAX_CHANNELS 100
#define BUF_SIZE 60000

static SHM_INFO Region;
static SHM_INFO OutRegion;
static char Buffer[BUF_SIZE];

static char RingName[MAX_RING_STR];
static char OutRingName[MAX_RING_STR];
static char MyModName[MAX_MOD_STR];
static int LogSwitch;
static long HeartBeatInterval;

static long RingKey;
static long OutRingKey;
static unsigned char InstId;
static unsigned char MyModId;
static unsigned char TypeHeartBeat;
static unsigned char TypeError;
static unsigned char TypeTraceBuf2;

#define ERR_TOOBIG 1
static char Text[150];

MSG_LOGO GetLogo[5];
short nLogo;
pid_t myPid;

typedef struct {
    char id[32];
    double fake_time;
} ClockEntry;

ClockEntry clocks[MAX_CHANNELS];
int nClocks = 0;

void template_config(char *);
void template_lookup(void);
void template_status(unsigned char, short, char *);
void template_tracebuf2(void);

int main(int argc, char **argv) {
    time_t timeNow, timeLastBeat;
    long recsize;
    MSG_LOGO reclogo;
    int res;
    char *lockfile;
    int lockfile_fd;

    if (argc != 2) {
        fprintf(stderr, "Uso: gps_timefix <archivo_configuracion>\n");
        exit(0);
    }

    logit_init(argv[1], 0, 256, 1);
    template_config(argv[1]);
    template_lookup();
    logit_init(argv[1], 0, 256, LogSwitch);

    lockfile = ew_lockfile_path(argv[1]); 
    if ((lockfile_fd = ew_lockfile(lockfile)) == -1) {
        fprintf(stderr, "Ya hay una instancia de %s ejecut\u00e1ndose.\n", argv[0]);
        exit(-1);
    }

    myPid = getpid();
    if (myPid == -1) {
        logit("e", "gps_timefix: No se pudo obtener PID.\n");
        exit(-1);
    }

    tport_attach(&Region, RingKey);
    tport_attach(&OutRegion, OutRingKey);
    logit("", "gps_timefix: Conectado al anillo %s (in) y %s (out)\n", RingName, OutRingName);

    while (tport_getmsg(&Region, GetLogo, nLogo, &reclogo, &recsize, Buffer, sizeof(Buffer) - 1) != GET_NONE);
    timeLastBeat = time(&timeNow) - HeartBeatInterval - 1;

    while (1) {
        if (time(&timeNow) - timeLastBeat >= HeartBeatInterval) {
            timeLastBeat = timeNow;
            template_status(TypeHeartBeat, 0, "");
        }

        do {
            if (tport_getflag(&Region) == TERMINATE || tport_getflag(&Region) == myPid) {
                tport_detach(&Region);
                tport_detach(&OutRegion);
                logit("et", "gps_timefix: Terminando ejecuci\u00f3n.\n");
                ew_unlockfile(lockfile_fd);
                ew_unlink_lockfile(lockfile);
                exit(0);
            }

            res = tport_getmsg(&Region, GetLogo, nLogo, &reclogo, &recsize, Buffer, sizeof(Buffer) - 1);
            if (res == GET_NONE) break;
            else if (res == GET_TOOBIG) {
                sprintf(Text, "Mensaje muy grande: %ld bytes", recsize);
                template_status(TypeError, ERR_TOOBIG, Text);
                continue;
            }

            Buffer[recsize] = '\0';
            if (reclogo.type == TypeTraceBuf2) {
                template_tracebuf2();
            }

        } while (res != GET_NONE);

        sleep_ew(1000);
    }
}

void template_tracebuf2(void) {
    TRACE2_HEADER *trh = (TRACE2_HEADER *) Buffer;
    char id[32];
    double duration;
    double *channel_time = NULL;
    int i;
    MSG_LOGO outlogo;

    if (trh->samprate <= 0 || trh->nsamp < 1) {
        logit("et", "gps_timefix: Datos inválidos: samprate=%.2f nsamp=%d\n", trh->samprate, trh->nsamp);
        return;
    }

    snprintf(id, sizeof(id), "%s.%s.%s.%s", trh->sta, trh->chan, trh->net, trh->loc);
    duration = (trh->nsamp - 1) / trh->samprate;

    for (i = 0; i < nClocks; i++) {
        if (strcmp(clocks[i].id, id) == 0) {
            channel_time = &clocks[i].fake_time;
            break;
        }
    }

    if (channel_time == NULL && nClocks < MAX_CHANNELS) {
        strcpy(clocks[nClocks].id, id);
        clocks[nClocks].fake_time = (double)time(NULL);
        channel_time = &clocks[nClocks].fake_time;
        logit("et", "gps_timefix: Inicializando fake_time (sistema) para %s: %.3f\n", id, *channel_time);
        nClocks++;
    }

    if (channel_time == NULL) {
        logit("et", "gps_timefix: Demasiados canales, no se puede seguir\n");
        return;
    }

    logit("et", "[ANTES] %s: start=%.3f end=%.3f\n", id, trh->starttime, trh->endtime);

    trh->starttime = *channel_time;
    trh->endtime = *channel_time + duration;
    *channel_time = trh->endtime;

    logit("et", "[DESPUÉS] %s: start=%.3f end=%.3f\n", id, trh->starttime, trh->endtime);

    outlogo.instid = InstId;
    outlogo.mod = MyModId;
    outlogo.type = TypeTraceBuf2;

    if (tport_putmsg(&OutRegion, &outlogo,
        sizeof(TRACE2_HEADER) + trh->nsamp * sizeof(float), Buffer) != PUT_OK) {
        logit("et", "gps_timefix: Error enviando mensaje corregido\n");
    }
}

void template_config(char *configfile) {
    int ncommand = 5;
    char init[10] = {0};
    int nfiles, i;
    char *com, *str;
    nLogo = 0;

    nfiles = k_open(configfile);
    if (nfiles == 0) {
        logit("e", "gps_timefix: No se puede abrir <%s>.\n", configfile);
        exit(-1);
    }

    while (nfiles > 0) {
        while (k_rd()) {
            com = k_str();
            if (!com || com[0] == '#') continue;

            if (k_its("LogFile")) {
                LogSwitch = k_int(); init[0] = 1;
            } else if (k_its("MyModuleId")) {
                str = k_str(); if (str) strcpy(MyModName, str); init[1] = 1;
            } else if (k_its("RingName")) {
                str = k_str(); if (str) strcpy(RingName, str); init[2] = 1;
            } else if (k_its("OutRingName")) {
                str = k_str(); if (str) strcpy(OutRingName, str); init[3] = 1;
            } else if (k_its("HeartBeatInterval")) {
                HeartBeatInterval = k_long(); init[4] = 1;
            } else if (k_its("GetEventsFrom")) {
                str = k_str(); GetInst(str, &GetLogo[nLogo].instid);
                str = k_str(); GetModId(str, &GetLogo[nLogo].mod);
                str = k_str(); GetType(str, &GetLogo[nLogo].type);
                nLogo++;
            }
        }
        nfiles = k_close();
    }

    for (i = 0; i < ncommand; i++) {
        if (!init[i]) {
            logit("e", "gps_timefix: Faltan parámetros obligatorios.\n");
            exit(-1);
        }
    }
}

void template_lookup(void) {
    if ((RingKey = GetKey(RingName)) == -1) {
        fprintf(stderr, "gps_timefix: RingName inválido <%s>\n", RingName);
        exit(-1);
    }
    if ((OutRingKey = GetKey(OutRingName)) == -1) {
        fprintf(stderr, "gps_timefix: OutRingName inválido <%s>\n", OutRingName);
        exit(-1);
    }
    if (GetLocalInst(&InstId) != 0) {
        fprintf(stderr, "gps_timefix: Error obteniendo InstId\n");
        exit(-1);
    }
    if (GetModId(MyModName, &MyModId) != 0) {
        fprintf(stderr, "gps_timefix: Módulo inválido <%s>\n", MyModName);
        exit(-1);
    }
    if (GetType("TYPE_HEARTBEAT", &TypeHeartBeat) != 0) {
        fprintf(stderr, "gps_timefix: Tipo TYPE_HEARTBEAT inválido\n");
        exit(-1);
    }
    if (GetType("TYPE_ERROR", &TypeError) != 0) {
        fprintf(stderr, "gps_timefix: Tipo TYPE_ERROR inválido\n");
        exit(-1);
    }
    if (GetType("TYPE_TRACEBUF2", &TypeTraceBuf2) != 0) {
        fprintf(stderr, "gps_timefix: Tipo TYPE_TRACEBUF2 inválido\n");
        exit(-1);
    }
}

void template_status(unsigned char type, short ierr, char *note) {
    MSG_LOGO logo;
    char msg[256];
    long size;
    time_t t;

    logo.instid = InstId;
    logo.mod = MyModId;
    logo.type = type;
    time(&t);

    if (type == TypeHeartBeat) {
        sprintf(msg, "%ld %ld\n", (long)t, (long)myPid);
    } else if (type == TypeError) {
        sprintf(msg, "%ld %hd %s\n", (long)t, ierr, note);
        logit("et", "gps_timefix: %s\n", note);
    }

    size = (long)strlen(msg);
    if (tport_putmsg(&Region, &logo, size, msg) != PUT_OK) {
        logit("et", "gps_timefix: Error enviando mensaje tipo %d.\n", type);
    }
}
