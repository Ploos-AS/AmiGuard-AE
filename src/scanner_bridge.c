#include <string.h>

#include "scanner_bridge.h"

#ifdef AMIGUARD_AE_WITH_AMIGUARD
#include "file_intake.h"
#endif

static AmiGuardAEScanFileProvider scan_file_provider = 0;

static void bridge_result(AmiGuardAEScanResult *out, AmiGuardAEScanStatus status, const char *detail)
{
    if (out == 0) return;
    out->status = status;
    if (detail == 0) detail = "";
    strncpy(out->detail, detail, AMIGUARD_AE_SCAN_DETAIL_MAX - 1);
    out->detail[AMIGUARD_AE_SCAN_DETAIL_MAX - 1] = '\0';
}

#ifdef AMIGUARD_AE_WITH_AMIGUARD
static int builtin_scan_file(const char *path, AmiGuardAEScanResult *out)
{
    struct amiguard_file_result result = amiguard_scan_file_readonly(path);

    if (result.status == AMIGUARD_FILE_INFECTED) {
        bridge_result(out, AMIGUARD_AE_SCAN_INFECTED, result.message);
        return 1;
    }
    if (result.status == AMIGUARD_FILE_TEST_SIGNATURE ||
        result.status == AMIGUARD_FILE_XVS_DETECTED ||
        result.status == AMIGUARD_FILE_MALFORMED_HUNK) {
        bridge_result(out, AMIGUARD_AE_SCAN_SUSPICIOUS, result.message);
        return 1;
    }
    if (result.status == AMIGUARD_FILE_VALID_HUNK ||
        result.status == AMIGUARD_FILE_NOT_HUNK) {
        bridge_result(out, AMIGUARD_AE_SCAN_CLEAN, result.message);
        return 1;
    }
    bridge_result(out, AMIGUARD_AE_SCAN_ERROR, result.message);
    return 0;
}
#endif

void amiguard_ae_scanner_set_provider(AmiGuardAEScanFileProvider provider)
{
    scan_file_provider = provider;
}

int amiguard_ae_scanner_available(void)
{
#ifdef AMIGUARD_AE_WITH_AMIGUARD
    return 1;
#else
    return scan_file_provider != 0;
#endif
}

int amiguard_ae_scanner_scan_file(const char *path, AmiGuardAEScanResult *out)
{
    if (out == 0) return 0;
    if (path == 0 || path[0] == '\0') {
        bridge_result(out, AMIGUARD_AE_SCAN_ERROR, "invalid path");
        return 0;
    }
    if (scan_file_provider != 0) return scan_file_provider(path, out);
#ifdef AMIGUARD_AE_WITH_AMIGUARD
    return builtin_scan_file(path, out);
#else
    bridge_result(out, AMIGUARD_AE_SCAN_ERROR, "scanner unavailable");
    return 0;
#endif
}
