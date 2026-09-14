#include "quarantine_model.h"
#include "checksum.h"

#include <stdio.h>
#include <string.h>

static void set_detail(char *detail, unsigned long size, const char *text)
{
    if (detail == 0 || size == 0) return;
    if (text == 0) text = "";
    strncpy(detail, text, (size_t)(size - 1));
    detail[size - 1] = '\0';
}

static int is_root_like(const char *path)
{
    const char *colon;
    if (strcmp(path, "/") == 0) return 1;
    colon = strchr(path, ':');
    if (colon != 0 && colon[1] == '\0') return 1;
    return 0;
}

int amiguard_ae_quarantine_path_safe(const char *path,
                                     char *detail,
                                     unsigned long detail_size)
{
    const char *p;
    if (path == 0 || path[0] == '\0') {
        set_detail(detail, detail_size, "path required");
        return 0;
    }
    if (strlen(path) >= AMIGUARD_AE_QUARANTINE_PATH_MAX) {
        set_detail(detail, detail_size, "path too long");
        return 0;
    }
    if (is_root_like(path)) {
        set_detail(detail, detail_size, "root/volume path forbidden");
        return 0;
    }
    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0) {
        set_detail(detail, detail_size, "relative directory path forbidden");
        return 0;
    }
    for (p = path; *p != '\0'; ++p) {
        if ((unsigned char)*p < 32U) {
            set_detail(detail, detail_size, "control character in path");
            return 0;
        }
    }
    set_detail(detail, detail_size, "path accepted for planning");
    return 1;
}

int amiguard_ae_quarantine_plan(const char *path,
                                AmiGuardAEQuarantinePlan *plan,
                                char *detail,
                                unsigned long detail_size)
{
    AmiGuardAEChecksumResult sum;
    FILE *fp;
    long size;
    unsigned long crc;

    if (plan == 0) {
        set_detail(detail, detail_size, "plan output required");
        return 0;
    }
    memset(plan, 0, sizeof(*plan));
    if (!amiguard_ae_quarantine_path_safe(path, detail, detail_size)) return 0;
    if (!amiguard_ae_checksum_crc32(path, &sum)) {
        set_detail(detail, detail_size, sum.detail);
        return 0;
    }
    if (sscanf(sum.checksum, "CRC32 %lx", &crc) != 1) {
        set_detail(detail, detail_size, "checksum parse failed");
        return 0;
    }
    fp = fopen(path, "rb");
    if (fp == 0) {
        set_detail(detail, detail_size, "source open failed");
        return 0;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        set_detail(detail, detail_size, "source size failed");
        return 0;
    }
    size = ftell(fp);
    fclose(fp);
    if (size < 0L) {
        set_detail(detail, detail_size, "source size failed");
        return 0;
    }
    plan->source_crc32 = crc;
    plan->source_size = (unsigned long)size;
    strncpy(plan->source, path, sizeof(plan->source) - 1);
    sprintf(plan->id, "Q%08lX%07lX", crc, plan->source_size & 0x0FFFFFFFUL);
    set_detail(detail, detail_size, "quarantine plan ready; no file changes made");
    return 1;
}
