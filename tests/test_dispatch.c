#include <stdio.h>
#include <string.h>

#include "arexx_dispatch.h"
#include "scanner_bridge.h"

static int expect(const char *command, long rc, const char *result)
{
    AmiGuardAERexxResult actual;
    amiguard_ae_arexx_dispatch(command, &actual);
    if (actual.rc != rc || strcmp(actual.result, result) != 0) {
        fprintf(stderr, "FAIL command=%s rc=%ld result=%s\n",
                command, actual.rc, actual.result);
        return 1;
    }
    return 0;
}

static int fake_scan(const char *path, AmiGuardAEScanResult *out)
{
    if (strcmp(path, "clean.bin") == 0) {
        out->status = AMIGUARD_AE_SCAN_CLEAN;
        strcpy(out->detail, "known-clean");
    } else if (strcmp(path, "virus.bin") == 0) {
        out->status = AMIGUARD_AE_SCAN_INFECTED;
        strcpy(out->detail, "Test.Virus");
    } else {
        out->status = AMIGUARD_AE_SCAN_SUSPICIOUS;
        strcpy(out->detail, "needs-analysis");
    }
    return 1;
}

int main(void)
{
    int failed = 0;
    failed += expect("PING", 0, "PONG");
    failed += expect(" version ", 0, "AmiGuard AE 0.1.0-m1");
    failed += expect("STATUS", 0, "READY M2.2 scanner=not-connected");
    failed += expect("HELP", 0, "PING VERSION STATUS HELP SCANFILE");
    failed += expect("SCANFILE", 10, "ERROR SCANFILE requires path");
    failed += expect("SCANFILE clean.bin", 10, "ERROR scanner unavailable");

    amiguard_ae_scanner_set_provider(fake_scan);
    failed += expect("STATUS", 0, "READY M2.2 scanner=connected");
    failed += expect("SCANFILE clean.bin", 0, "CLEAN known-clean");
    failed += expect("SCANFILE virus.bin", 5, "INFECTED Test.Virus");
    failed += expect("SCANFILE sample.bin", 5, "SUSPICIOUS needs-analysis");
    failed += expect("BOGUS", 10, "ERROR unknown command");
    failed += expect("", 10, "ERROR empty command");
    if (failed != 0) return 1;
    puts("M2.2 scanner bridge qualification: PASS");
    return 0;
}
