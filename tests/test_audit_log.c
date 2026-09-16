#include <stdio.h>
#include <string.h>
#include "audit_log.h"

int main(void)
{
    char detail[AMIGUARD_AE_AUDIT_DETAIL_MAX];
    char line[512];
    FILE *fp;
    const char *path = "build/m4_5_audit.log";

    remove(path);
    if (amiguard_ae_audit_available()) return 1;
    if (amiguard_ae_audit_append("SCAN", "CLEAN", "file.bin", "ok",
                                 detail, sizeof(detail))) return 1;
    if (!amiguard_ae_audit_set_path(path, detail, sizeof(detail))) return 1;
    if (!amiguard_ae_audit_available()) return 1;
    if (!amiguard_ae_audit_append("SCAN", "CLEAN", "file.bin", "crc=1234",
                                  detail, sizeof(detail))) return 1;
    if (!amiguard_ae_audit_append("QUARANTINE", "COMMITTED", "Q123", "source=file.bin",
                                  detail, sizeof(detail))) return 1;
    if (amiguard_ae_audit_append("BAD|EVENT", "FAIL", "x", "x",
                                 detail, sizeof(detail))) return 1;

    fp = fopen(path, "rb");
    if (fp == 0) return 1;
    if (fgets(line, sizeof(line), fp) == 0 ||
        strcmp(line, "AMIGUARD-AUDIT|1|SCAN|CLEAN|file.bin|crc=1234\n") != 0) {
        fclose(fp); return 1;
    }
    if (fgets(line, sizeof(line), fp) == 0 ||
        strcmp(line, "AMIGUARD-AUDIT|1|QUARANTINE|COMMITTED|Q123|source=file.bin\n") != 0) {
        fclose(fp); return 1;
    }
    if (fgets(line, sizeof(line), fp) != 0) { fclose(fp); return 1; }
    fclose(fp);
    remove(path);
    puts("M4.5 audit log foundation: PASS");
    return 0;
}
