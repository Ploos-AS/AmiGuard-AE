#include <stdio.h>
#include <string.h>

#include "arexx_dispatch.h"
#include "scanner_bridge.h"
#include "signature_bridge.h"

static int expect(const char *command, long rc, const char *result)
{
    AmiGuardAERexxResult actual;
    amiguard_ae_arexx_dispatch(command, &actual);
    if (actual.rc != rc || strcmp(actual.result, result) != 0) {
        fprintf(stderr, "FAIL command=%s rc=%ld result=%s\n", command, actual.rc, actual.result);
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

static unsigned long fake_signature_count(void)
{
    return 2UL;
}

static int fake_signature_info(unsigned long index, AmiGuardAESignatureInfo *out)
{
    if (out == 0 || index >= 2UL) return 0;
    if (index == 0UL) {
        strcpy(out->type, "BOOTBLOCK");
        strcpy(out->name, "Test.Boot");
        out->offset = 64UL;
        out->length = 8UL;
        out->test_only = 0;
    } else {
        strcpy(out->type, "FILE");
        strcpy(out->name, "Test.File");
        out->offset = 4UL;
        out->length = 18UL;
        out->test_only = 1;
    }
    return 1;
}

static int write_file(const char *path, const unsigned char *data, unsigned long size)
{
    FILE *fp = fopen(path, "wb");
    if (fp == 0) return 0;
    if (fwrite(data, 1U, (size_t)size, fp) != (size_t)size) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

int main(void)
{
    static const unsigned char checksum_data[] = "123456789";
    static const unsigned char hunk_data[] = {0x00U,0x00U,0x03U,0xF3U};
    static const unsigned char iff_data[] = {'F','O','R','M'};
    int failed = 0;

    if (!write_file("build/checksum-fixture.bin", checksum_data, 9UL) ||
        !write_file("build/hunk-fixture.bin", hunk_data, 4UL) ||
        !write_file("build/iff-fixture.bin", iff_data, 4UL)) {
        fprintf(stderr, "FAIL cannot create fixtures\n");
        return 1;
    }

    failed += expect("PING", 0, "PONG");
    failed += expect(" version ", 0, "AmiGuard AE 0.3.0-m3.2");
    failed += expect("STATUS", 0, "READY M3.2 scanner=not-connected");
    failed += expect("HELP", 0, "PING VERSION STATUS HELP SCAN SCANFILE CHECKSUM IDENTIFY SIGNATURE.COUNT SIGNATURE.INFO RESULT.STATUS RESULT.PATH RESULT.DETAIL RESULT.CLEAR");
    failed += expect("SIGNATURE.COUNT", 10, "ERROR signature backend unavailable");
    failed += expect("SIGNATURE.INFO 0", 10, "ERROR signature backend unavailable");
    amiguard_ae_signature_set_count_provider(fake_signature_count);
    amiguard_ae_signature_set_info_provider(fake_signature_info);
    failed += expect("SIGNATURE.COUNT", 0, "2");
    failed += expect("SIGNATURE.INFO", 10, "ERROR SIGNATURE.INFO requires index");
    failed += expect("SIGNATURE.INFO x", 10, "ERROR invalid signature index");
    failed += expect("SIGNATURE.INFO 2", 10, "ERROR invalid signature index");
    failed += expect("SIGNATURE.INFO 0", 0, "INDEX=0 TYPE=BOOTBLOCK OFFSET=64 LENGTH=8 TEST_ONLY=0 NAME=Test.Boot");
    failed += expect("SIGNATURE.INFO 1", 0, "INDEX=1 TYPE=FILE OFFSET=4 LENGTH=18 TEST_ONLY=1 NAME=Test.File");
    failed += expect("IDENTIFY", 10, "ERROR IDENTIFY requires path");
    failed += expect("IDENTIFY build/hunk-fixture.bin", 0, "AMIGA-HUNK HUNK_HEADER");
    failed += expect("IDENTIFY build/iff-fixture.bin", 0, "IFF FORM container");
    failed += expect("IDENTIFY build/checksum-fixture.bin", 0, "DATA unrecognized binary/data");
    failed += expect("IDENTIFY build/missing.bin", 10, "ERROR cannot open file read-only");
    failed += expect("CHECKSUM", 10, "ERROR CHECKSUM requires path");
    failed += expect("CHECKSUM build/checksum-fixture.bin", 0, "CRC32 CBF43926");
    failed += expect("RESULT.STATUS", 10, "ERROR no scan result");
    failed += expect("SCAN", 10, "ERROR SCAN requires path");
    failed += expect("SCAN clean.bin", 10, "ERROR scanner unavailable");
    failed += expect("RESULT.STATUS", 0, "ERROR");
    failed += expect("RESULT.PATH", 0, "clean.bin");
    failed += expect("RESULT.DETAIL", 0, "scanner unavailable");
    failed += expect("RESULT.CLEAR", 0, "OK");
    failed += expect("SCANFILE", 10, "ERROR SCANFILE requires path");

    amiguard_ae_scanner_set_provider(fake_scan);
    failed += expect("STATUS", 0, "READY M3.2 scanner=connected");
    failed += expect("SCAN clean.bin", 0, "CLEAN known-clean");
    failed += expect("RESULT.STATUS", 0, "CLEAN");
    failed += expect("RESULT.PATH", 0, "clean.bin");
    failed += expect("SCAN virus.bin", 5, "INFECTED Test.Virus");
    failed += expect("RESULT.STATUS", 0, "INFECTED");
    failed += expect("SCAN sample.bin", 5, "SUSPICIOUS needs-analysis");
    failed += expect("RESULT.STATUS", 0, "SUSPICIOUS");
    failed += expect("SCANFILE clean.bin", 0, "CLEAN known-clean");
    failed += expect("SCANFILE virus.bin", 5, "INFECTED Test.Virus");
    failed += expect("SCANFILE sample.bin", 5, "SUSPICIOUS needs-analysis");
    failed += expect("RESULT.UNKNOWN", 10, "ERROR unknown RESULT command");
    failed += expect("BOGUS", 10, "ERROR unknown command");
    failed += expect("", 10, "ERROR empty command");

    remove("build/checksum-fixture.bin");
    remove("build/hunk-fixture.bin");
    remove("build/iff-fixture.bin");
    if (failed != 0) return 1;
    puts("M3.2 signature info qualification: PASS");
    return 0;
}
