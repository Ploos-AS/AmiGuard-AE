#include <stdio.h>
#include <string.h>
#include "amiguard_ae.h"
#include "arexx_dispatch.h"

static int expect(const char *command, long rc, const char *result)
{
    AmiGuardAERexxResult actual;
    amiguard_ae_arexx_dispatch(command, &actual);
    if (actual.rc != rc || strcmp(actual.result, result) != 0)
        return 0;
    printf("SMOKE PASS %s => %s\n", command, actual.result);
    return 1;
}

static int expect_prefix(const char *command, long rc, const char *prefix)
{
    AmiGuardAERexxResult actual;
    unsigned long n = (unsigned long)strlen(prefix);
    amiguard_ae_arexx_dispatch(command, &actual);
    if (actual.rc != rc || strncmp(actual.result, prefix, n) != 0)
        return 0;
    printf("SMOKE PASS %s => %s\n", command, actual.result);
    return 1;
}

static int write_file(const char *path, const unsigned char *data, unsigned long size)
{
    FILE *fp = fopen(path, "wb");
    if (fp == 0)
        return 0;
    if (fwrite(data, 1U, (size_t)size, fp) != (size_t)size) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

int main(void)
{
    static const unsigned char crc[] = "123456789";
    static const unsigned char hunk[] = {0,0,3,0xF3};
    static const unsigned char sigdb[] =
        "AMIGUARD-FILE-SIGDB 1\n"
        "FILE|AROS.Runtime.Test|0|1|313233343536373839|ffffffffffffffffff\n";
    static const unsigned char manifest[] = "AMIGUARD-SIG-MANIFEST 1\n";
    int ok = 1;

    if (!write_file("RAM:amiguard-ae-crc.bin",crc,9UL) ||
        !write_file("RAM:amiguard-ae-hunk.bin",hunk,4UL) ||
        !write_file("RAM:amiguard-ae-runtime.sigdb",sigdb,(unsigned long)(sizeof(sigdb)-1U)) ||
        !write_file("RAM:amiguard-ae-runtime.manifest",manifest,(unsigned long)(sizeof(manifest)-1U)))
        return AMIGUARD_AE_RC_FAIL;

    ok = expect("PING",0,"PONG") && ok;
    ok = expect("STATUS",0,"READY M3.7 scanner=connected") && ok;
    ok = expect("SIGNATURE.STATUS",0,"READY COUNT=4 UPDATE=AVAILABLE VERIFY=CRC32 AUTH=UNAVAILABLE") && ok;
    ok = expect("SIGNATURE.UPDATE RAM:amiguard-ae-runtime.sigdb 00000000 RAM:amiguard-ae-runtime.manifest",10,"ERROR checksum mismatch expected=00000000 actual=4D0F09D8") && ok;
    ok = expect("SIGNATURE.UPDATE RAM:amiguard-ae-runtime.sigdb 4D0F09D8 RAM:amiguard-ae-runtime.manifest",10,"ERROR signature authenticator unavailable") && ok;
    ok = expect("SIGNATURE.COUNT",0,"4") && ok;
    ok = expect("SIGNATURE.STATUS",0,"READY COUNT=4 UPDATE=AVAILABLE VERIFY=CRC32 AUTH=UNAVAILABLE") && ok;
    ok = expect_prefix("SCAN RAM:amiguard-ae-crc.bin",0,"CLEAN ") && ok;
    ok = expect("RESULT.STATUS",0,"CLEAN") && ok;
    ok = expect("CHECKSUM RAM:amiguard-ae-crc.bin",0,"CRC32 CBF43926") && ok;
    ok = expect("IDENTIFY RAM:amiguard-ae-hunk.bin",0,"AMIGA-HUNK HUNK_HEADER") && ok;
    ok = expect("RESULT.CLEAR",0,"OK") && ok;

    remove("RAM:amiguard-ae-crc.bin");
    remove("RAM:amiguard-ae-hunk.bin");
    remove("RAM:amiguard-ae-runtime.sigdb");
    remove("RAM:amiguard-ae-runtime.manifest");

    if (!ok)
        return AMIGUARD_AE_RC_FAIL;
    puts("M3.7 AROS authenticated update gate smoke: PASS");
    return 0;
}
