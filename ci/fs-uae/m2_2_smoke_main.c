#include <stdio.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"

static int expect(const char *command, long expected_rc, const char *expected_result)
{
    AmiGuardAERexxResult result;
    amiguard_ae_arexx_dispatch(command, &result);
    if (result.rc != expected_rc) return 0;
    if (strcmp(result.result, expected_result) != 0) return 0;
    printf("SMOKE PASS %s => %s\n", command, result.result);
    return 1;
}

static int write_checksum_fixture(void)
{
    FILE *fp = fopen("RAM:amiguard-ae-crc.bin", "wb");
    if (fp == 0) return 0;
    if (fwrite("123456789", 1U, 9U, fp) != 9U) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

int main(void)
{
    int ok = 1;
    if (!write_checksum_fixture()) return AMIGUARD_AE_RC_FAIL;
    ok = expect("PING", AMIGUARD_AE_RC_OK, "PONG") && ok;
    ok = expect("STATUS", AMIGUARD_AE_RC_OK, "READY M2.4 scanner=connected") && ok;
    ok = expect("CHECKSUM RAM:amiguard-ae-crc.bin", AMIGUARD_AE_RC_OK, "CRC32 CBF43926") && ok;
    ok = expect("RESULT.STATUS", AMIGUARD_AE_RC_ERROR, "ERROR no scan result") && ok;
    ok = expect("RESULT.CLEAR", AMIGUARD_AE_RC_OK, "OK") && ok;
    ok = expect("BOGUS", AMIGUARD_AE_RC_ERROR, "ERROR unknown command") && ok;
    remove("RAM:amiguard-ae-crc.bin");
    if (!ok) return AMIGUARD_AE_RC_FAIL;
    puts("M2.4 AROS checksum smoke: PASS");
    return AMIGUARD_AE_RC_OK;
}
