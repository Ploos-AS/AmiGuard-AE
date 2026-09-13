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

static int write_file(const char *path, const unsigned char *data, unsigned long size)
{
    FILE *fp = fopen(path, "wb");
    if (fp == 0) return 0;
    if (fwrite(data, 1U, (size_t)size, fp) != (size_t)size) { fclose(fp); return 0; }
    fclose(fp);
    return 1;
}

int main(void)
{
    static const unsigned char crc_data[] = "123456789";
    static const unsigned char hunk_data[] = {0x00U,0x00U,0x03U,0xF3U};
    int ok = 1;

    if (!write_file("RAM:amiguard-ae-crc.bin", crc_data, 9UL)) return AMIGUARD_AE_RC_FAIL;
    if (!write_file("RAM:amiguard-ae-hunk.bin", hunk_data, 4UL)) return AMIGUARD_AE_RC_FAIL;

    ok = expect("PING", AMIGUARD_AE_RC_OK, "PONG") && ok;
    ok = expect("STATUS", AMIGUARD_AE_RC_OK, "READY M2.5 scanner=connected") && ok;
    ok = expect("CHECKSUM RAM:amiguard-ae-crc.bin", AMIGUARD_AE_RC_OK, "CRC32 CBF43926") && ok;
    ok = expect("IDENTIFY RAM:amiguard-ae-hunk.bin", AMIGUARD_AE_RC_OK, "AMIGA-HUNK HUNK_HEADER") && ok;
    ok = expect("RESULT.STATUS", AMIGUARD_AE_RC_ERROR, "ERROR no scan result") && ok;
    ok = expect("RESULT.CLEAR", AMIGUARD_AE_RC_OK, "OK") && ok;
    ok = expect("BOGUS", AMIGUARD_AE_RC_ERROR, "ERROR unknown command") && ok;

    remove("RAM:amiguard-ae-crc.bin");
    remove("RAM:amiguard-ae-hunk.bin");
    if (!ok) return AMIGUARD_AE_RC_FAIL;
    puts("M2.5 AROS identify smoke: PASS");
    return AMIGUARD_AE_RC_OK;
}
