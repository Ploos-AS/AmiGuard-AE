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
    failed += expect(" version ", 0, "AmiGuard AE 0.2.0-m2.3");
    failed += expect("STATUS", 0, "READY M2.3 scanner=not-connected");
    failed += expect("RESULT.STATUS", 10, "ERROR no scan result");
    failed += expect("HELP", 0, "PING VERSION STATUS HELP SCANFILE RESULT.STATUS RESULT.PATH RESULT.DETAIL RESULT.CLEAR");
    failed += expect("SCANFILE", 10, "ERROR SCANFILE requires path");
    failed += expect("SCANFILE clean.bin", 10, "ERROR scanner unavailable");
    failed += expect("RESULT.STATUS", 0, "ERROR");
    failed += expect("RESULT.PATH", 0, "clean.bin");
    failed += expect("RESULT.DETAIL", 0, "scanner unavailable");
    failed += expect("RESULT.CLEAR", 0, "OK");
    failed += expect("RESULT.STATUS", 10, "ERROR no scan result");

    amiguard_ae_scanner_set_provider(fake_scan);
    failed += expect("STATUS", 0, "READY M2.3 scanner=connected");
    failed += expect("SCANFILE clean.bin", 0, "CLEAN known-clean");
    failed += expect("RESULT.STATUS", 0, "CLEAN");
    failed += expect("RESULT.PATH", 0, "clean.bin");
    failed += expect("RESULT.DETAIL", 0, "known-clean");
    failed += expect("SCANFILE virus.bin", 5, "INFECTED Test.Virus");
    failed += expect("RESULT.STATUS", 0, "INFECTED");
    failed += expect("RESULT.DETAIL", 0, "Test.Virus");
    failed += expect("SCANFILE sample.bin", 5, "SUSPICIOUS needs-analysis");
    failed += expect("RESULT.STATUS", 0, "SUSPICIOUS");
    failed += expect("RESULT.UNKNOWN", 10, "ERROR unknown RESULT command");
    failed += expect("BOGUS", 10, "ERROR unknown command");
    failed += expect("", 10, "ERROR empty command");
    if (failed != 0) return 1;
    puts("M2.3 structured result qualification: PASS");
    return 0;
}
