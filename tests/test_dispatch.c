#include <stdio.h>
#include <string.h>
#include "arexx_dispatch.h"
#include "scanner_bridge.h"
#include "signature_bridge.h"
#include "quarantine_model.h"

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

static int exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == 0) return 0;
    fclose(fp);
    return 1;
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
    if (out == 0 || index >= 2UL)
        return 0;
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

static int fake_signature_update(const char *path, char *detail, unsigned long detail_size)
{
    if (path == 0 || strcmp(path, "build/fixture.sigdb") != 0)
        return 0;
    if (detail == 0 || detail_size < 10UL)
        return 0;
    strcpy(detail, "fixture-v1");
    return 1;
}

static int fake_signature_auth(const char *manifest_path,
                               const char *database_path,
                               const char *crc32,
                               char *detail,
                               unsigned long detail_size)
{
    if (detail == 0 || detail_size < 8UL)
        return 0;
    if (manifest_path == 0 || strcmp(manifest_path, "build/fixture.manifest") != 0 ||
        database_path == 0 || strcmp(database_path, "build/fixture.sigdb") != 0 ||
        crc32 == 0 || strcmp(crc32, "CBF43926") != 0) {
        strcpy(detail, "manifest authentication failed");
        return 0;
    }
    strcpy(detail, "manifest-ok");
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
    static const unsigned char checksum_data[] = "123456789";
    static const unsigned char manifest_data[] = "fixture manifest";
    static const unsigned char quarantine_data[] = "M4.4 quarantine/restore dispatcher fixture";
    static const unsigned char hunk_data[] = {0x00U,0x00U,0x03U,0xF3U};
    static const unsigned char iff_data[] = {'F','O','R','M'};
    AmiGuardAEQuarantinePlan qplan;
    char qdetail[AMIGUARD_AE_QUARANTINE_DETAIL_MAX];
    char qcommand[AMIGUARD_AE_QUARANTINE_PATH_MAX + 16];
    char rcommand[AMIGUARD_AE_QUARANTINE_ID_MAX + 16];
    char qobject[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char qmetadata[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char qexpected[AMIGUARD_AE_RESULT_MAX];
    char rexpected[AMIGUARD_AE_RESULT_MAX];
    int failed = 0;

    if (!write_file("build/checksum-fixture.bin", checksum_data, 9UL) ||
        !write_file("build/fixture.sigdb", checksum_data, 9UL) ||
        !write_file("build/fixture.manifest", manifest_data, (unsigned long)(sizeof(manifest_data) - 1U)) ||
        !write_file("build/hunk-fixture.bin", hunk_data, 4UL) ||
        !write_file("build/iff-fixture.bin", iff_data, 4UL))
        return 1;

    failed += expect("PING",0,"PONG");
    failed += expect(" version ",0,"AmiGuard AE 0.4.0-m4.4");
    failed += expect("STATUS",0,"READY M4.4 scanner=not-connected");
    failed += expect("SIGNATURE.STATUS",10,"ERROR signature backend unavailable");
    failed += expect("SIGNATURE.UPDATE",10,"ERROR SIGNATURE.UPDATE requires path CRC32 manifest");
    failed += expect("QUARANTINE",10,"ERROR QUARANTINE requires path");
    failed += expect("RESTORE",10,"ERROR RESTORE requires id");
    failed += expect("QUARANTINE build/m4_4_quarantine.bin",10,"ERROR quarantine directory unavailable");

    amiguard_ae_signature_set_count_provider(fake_signature_count);
    amiguard_ae_signature_set_info_provider(fake_signature_info);
    amiguard_ae_signature_set_update_provider(fake_signature_update);
    failed += expect("SIGNATURE.STATUS",0,"READY COUNT=2 UPDATE=AVAILABLE VERIFY=CRC32 AUTH=UNAVAILABLE");
    failed += expect("SIGNATURE.UPDATE build/fixture.sigdb CBF43926 build/fixture.manifest",10,"ERROR signature authenticator unavailable");

    amiguard_ae_signature_set_auth_provider(fake_signature_auth);
    failed += expect("SIGNATURE.STATUS",0,"READY COUNT=2 UPDATE=AVAILABLE VERIFY=CRC32 AUTH=AVAILABLE");
    failed += expect("SIGNATURE.UPDATE build/fixture.sigdb 00000000 build/fixture.manifest",10,"ERROR checksum mismatch expected=00000000 actual=CBF43926");
    failed += expect("SIGNATURE.UPDATE build/fixture.sigdb CBF43926 build/bad.manifest",10,"ERROR manifest authentication failed");
    failed += expect("SIGNATURE.UPDATE build/fixture.sigdb cbf43926 build/fixture.manifest",0,"UPDATED VERIFIED=CRC32 AUTH=CUSTOM fixture-v1");
    failed += expect("SIGNATURE.COUNT",0,"2");
    failed += expect("SIGNATURE.INFO 0",0,"INDEX=0 TYPE=BOOTBLOCK OFFSET=64 LENGTH=8 TEST_ONLY=0 NAME=Test.Boot");
    failed += expect("CHECKSUM build/checksum-fixture.bin",0,"CRC32 CBF43926");
    failed += expect("IDENTIFY build/hunk-fixture.bin",0,"AMIGA-HUNK HUNK_HEADER");
    failed += expect("IDENTIFY build/iff-fixture.bin",0,"IFF FORM container");
    failed += expect("RESULT.STATUS",10,"ERROR no scan result");
    failed += expect("SCAN",10,"ERROR SCAN requires path");

    amiguard_ae_scanner_set_provider(fake_scan);
    failed += expect("STATUS",0,"READY M4.4 scanner=connected");
    failed += expect("SCAN clean.bin",0,"CLEAN known-clean");
    failed += expect("RESULT.STATUS",0,"CLEAN");
    failed += expect("SCAN virus.bin",5,"INFECTED Test.Virus");
    failed += expect("RESULT.CLEAR",0,"OK");

    if (!write_file("build/m4_4_quarantine.bin", quarantine_data,
                    (unsigned long)(sizeof(quarantine_data) - 1U)))
        return 1;
    if (!amiguard_ae_quarantine_plan("build/m4_4_quarantine.bin", &qplan,
                                     qdetail, sizeof(qdetail)))
        return 1;
    sprintf(qobject, "build/%s.qtn", qplan.id);
    sprintf(qmetadata, "build/%s.meta", qplan.id);
    remove(qobject);
    remove(qmetadata);
    sprintf(qcommand, "QUARANTINE %s", qplan.source);
    sprintf(qexpected, "QUARANTINED ID=%s PATH=%s", qplan.id, qobject);
    if (!amiguard_ae_quarantine_set_directory("build", qdetail, sizeof(qdetail)))
        return 1;
    failed += expect(qcommand, 0, qexpected);
    if (exists("build/m4_4_quarantine.bin"))
        failed += 1;
    if (!exists(qobject) || !exists(qmetadata))
        failed += 1;

    sprintf(rcommand, "RESTORE %s", qplan.id);
    sprintf(rexpected, "RESTORED ID=%s PATH=%s QUARANTINE=RETAINED", qplan.id, qplan.source);
    failed += expect(rcommand, 0, rexpected);
    if (!exists("build/m4_4_quarantine.bin"))
        failed += 1;
    if (!exists(qobject) || !exists(qmetadata))
        failed += 1;
    failed += expect(rcommand, 10, "ERROR restore destination exists; refusing overwrite");

    failed += expect("BOGUS",10,"ERROR unknown command");
    failed += expect("",10,"ERROR unknown command");

    remove("build/checksum-fixture.bin");
    remove("build/fixture.sigdb");
    remove("build/fixture.manifest");
    remove("build/hunk-fixture.bin");
    remove("build/iff-fixture.bin");
    remove("build/m4_4_quarantine.bin");
    remove(qobject);
    remove(qmetadata);
    amiguard_ae_quarantine_set_directory(0, qdetail, sizeof(qdetail));

    if (failed != 0)
        return 1;
    puts("M4.4 restore dispatcher qualification: PASS");
    return 0;
}
