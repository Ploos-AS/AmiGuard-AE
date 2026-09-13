#include <stdio.h>

#include "amiguard_provider.h"
#include "scanner_bridge.h"

int main(void)
{
    const char *path = "build/m2_2_fixture.bin";
    FILE *fp;
    AmiGuardAEScanResult result;

    if (!amiguard_ae_amiguard_provider_install()) return 1;

    fp = fopen(path, "wb");
    if (fp == 0) return 1;
    fputs("AmiGuard AE M2.2 fixture\n", fp);
    fclose(fp);

    if (!amiguard_ae_scanner_scan_file(path, &result)) return 1;
    if (result.status != AMIGUARD_AE_SCAN_CLEAN) return 1;

    remove(path);
    puts("M2.2 AmiGuard provider integration: PASS");
    return 0;
}
