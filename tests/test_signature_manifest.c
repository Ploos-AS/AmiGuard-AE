#include <stdio.h>
#include <string.h>

#include "signature_manifest.h"

static int write_text(const char *path, const char *text)
{
    FILE *fp = fopen(path, "wb");
    if (fp == 0)
        return 0;
    if (fputs(text, fp) == EOF) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int expect(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        return 1;
    }
    return 0;
}

int main(void)
{
    static const char *valid =
        "AMIGUARD-SIGMANIFEST 1\n"
        "ALGORITHM=ED25519\n"
        "KEYID=ploos-release-1\n"
        "SEQUENCE=42\n"
        "DATABASE=amiguard-file.db\n"
        "CRC32=4D0F09D8\n"
        "SIGNATURE=00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n";
    AmiGuardAESignatureManifest manifest;
    char detail[AMIGUARD_AE_MANIFEST_DETAIL_MAX];
    int failed = 0;

    if (!write_text("build/manifest-valid.txt", valid))
        return 1;
    failed += expect(amiguard_ae_signature_manifest_load("build/manifest-valid.txt", &manifest, detail, sizeof(detail)) == 1, "valid manifest loads");
    failed += expect(strcmp(manifest.algorithm, "ED25519") == 0, "algorithm");
    failed += expect(strcmp(manifest.key_id, "ploos-release-1") == 0, "key id");
    failed += expect(manifest.sequence == 42UL, "sequence");
    failed += expect(strcmp(manifest.database, "amiguard-file.db") == 0, "database");
    failed += expect(strcmp(manifest.crc32, "4D0F09D8") == 0, "crc32");
    failed += expect(strlen(manifest.signature_hex) == 128U, "signature length");

    if (!write_text("build/manifest-invalid.txt",
                    "AMIGUARD-SIGMANIFEST 1\nALGORITHM=HMAC\n"))
        return 1;
    failed += expect(amiguard_ae_signature_manifest_load("build/manifest-invalid.txt", &manifest, detail, sizeof(detail)) == 0, "unsupported algorithm rejected");

    remove("build/manifest-valid.txt");
    remove("build/manifest-invalid.txt");
    if (failed != 0)
        return 1;
    puts("M3.8a signed manifest parser qualification: PASS");
    return 0;
}
