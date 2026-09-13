#include <string.h>

#include "result_store.h"

typedef struct AmiGuardAELastResult {
    int valid;
    AmiGuardAEScanStatus status;
    char path[AMIGUARD_AE_RESULT_PATH_MAX];
    char detail[AMIGUARD_AE_RESULT_DETAIL_MAX];
} AmiGuardAELastResult;

static AmiGuardAELastResult last_result;

static void copy_text(char *dst, unsigned long size, const char *src)
{
    if (size == 0) return;
    if (src == 0) src = "";
    strncpy(dst, src, size - 1);
    dst[size - 1] = '\0';
}

void amiguard_ae_result_clear(void)
{
    memset(&last_result, 0, sizeof(last_result));
}

void amiguard_ae_result_record(const char *path, const AmiGuardAEScanResult *scan)
{
    if (scan == 0) {
        amiguard_ae_result_clear();
        return;
    }
    last_result.valid = 1;
    last_result.status = scan->status;
    copy_text(last_result.path, sizeof(last_result.path), path);
    copy_text(last_result.detail, sizeof(last_result.detail), scan->detail);
}

int amiguard_ae_result_valid(void)
{
    return last_result.valid;
}

const char *amiguard_ae_result_status(void)
{
    if (!last_result.valid) return "NONE";
    if (last_result.status == AMIGUARD_AE_SCAN_CLEAN) return "CLEAN";
    if (last_result.status == AMIGUARD_AE_SCAN_INFECTED) return "INFECTED";
    if (last_result.status == AMIGUARD_AE_SCAN_SUSPICIOUS) return "SUSPICIOUS";
    return "ERROR";
}

const char *amiguard_ae_result_path(void)
{
    return last_result.valid ? last_result.path : "";
}

const char *amiguard_ae_result_detail(void)
{
    return last_result.valid ? last_result.detail : "";
}
