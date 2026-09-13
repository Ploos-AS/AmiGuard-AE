#include <string.h>

#include "amiguard_provider.h"
#include "scanner_bridge.h"
#include "file_intake.h"

static void set_detail(AmiGuardAEScanResult *out, AmiGuardAEScanStatus status, const char *detail)
{
    if (out == 0) return;
    out->status = status;
    if (detail == 0) detail = "";
    strncpy(out->detail, detail, AMIGUARD_AE_SCAN_DETAIL_MAX - 1);
    out->detail[AMIGUARD_AE_SCAN_DETAIL_MAX - 1] = '\0';
}

static int amiguard_scanfile_provider(const char *path, AmiGuardAEScanResult *out)
{
    struct amiguard_file_result result = amiguard_scan_file_readonly(path);

    switch (result.status) {
        case AMIGUARD_FILE_INFECTED:
            set_detail(out, AMIGUARD_AE_SCAN_INFECTED, result.message);
            return 1;
        case AMIGUARD_FILE_TEST_SIGNATURE:
            set_detail(out, AMIGUARD_AE_SCAN_SUSPICIOUS, result.message);
            return 1;
        case AMIGUARD_FILE_XVS_DETECTED:
            set_detail(out, AMIGUARD_AE_SCAN_SUSPICIOUS, result.message);
            return 1;
        case AMIGUARD_FILE_MALFORMED_HUNK:
            set_detail(out, AMIGUARD_AE_SCAN_SUSPICIOUS, result.message);
            return 1;
        case AMIGUARD_FILE_VALID_HUNK:
        case AMIGUARD_FILE_NOT_HUNK:
            set_detail(out, AMIGUARD_AE_SCAN_CLEAN, result.message);
            return 1;
        case AMIGUARD_FILE_ERROR:
        default:
            set_detail(out, AMIGUARD_AE_SCAN_ERROR, result.message);
            return 0;
    }
}

int amiguard_ae_amiguard_provider_install(void)
{
    amiguard_ae_scanner_set_provider(amiguard_scanfile_provider);
    return amiguard_ae_scanner_available();
}
