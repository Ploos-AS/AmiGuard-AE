#include <string.h>

#include "scanner_bridge.h"

static AmiGuardAEScanFileProvider scan_file_provider = 0;

static void bridge_result(AmiGuardAEScanResult *out, AmiGuardAEScanStatus status, const char *detail)
{
    if (out == 0) return;
    out->status = status;
    if (detail == 0) detail = "";
    strncpy(out->detail, detail, AMIGUARD_AE_SCAN_DETAIL_MAX - 1);
    out->detail[AMIGUARD_AE_SCAN_DETAIL_MAX - 1] = '\0';
}

void amiguard_ae_scanner_set_provider(AmiGuardAEScanFileProvider provider)
{
    scan_file_provider = provider;
}

int amiguard_ae_scanner_available(void)
{
    return scan_file_provider != 0;
}

int amiguard_ae_scanner_scan_file(const char *path, AmiGuardAEScanResult *out)
{
    if (out == 0) return 0;
    if (path == 0 || path[0] == '\0') {
        bridge_result(out, AMIGUARD_AE_SCAN_ERROR, "invalid path");
        return 0;
    }
    if (scan_file_provider == 0) {
        bridge_result(out, AMIGUARD_AE_SCAN_ERROR, "scanner unavailable");
        return 0;
    }
    return scan_file_provider(path, out);
}
