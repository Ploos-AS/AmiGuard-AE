#include <stdio.h>
#include <string.h>

#include "signature_auth_ed25519.h"

static int write_text(const char *path, const char *text)
{
    FILE *fp = fopen(path, "wb");
    size_t n;
    if (fp == 0)
        return 0;
    n = strlen(text);
    if (fwrite(text, 1U, n, fp) != n) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int expect_auth(const char *manifest, int expected, const char *detail_prefix)
{
    char detail[128];
    int actual = amiguard_ae_ed25519_authenticate(manifest,
                                                   "build/fixture.sigdb",
                                                   "CBF43926",
                                                   detail,
                                                   sizeof(detail));
    if (actual != expected || strncmp(detail, detail_prefix, strlen(detail_prefix)) != 0) {
        fprintf(stderr, "FAIL auth=%d detail=%s\n", actual, detail);
        return 0;
    }
    return 1;
}

int main(void)
{
    static const char manifest1[] =
        "AMIGUARD-SIGMANIFEST 1\n"
        "ALGORITHM=ED25519\n"
        "KEYID=ci-test-1\n"
        "SEQUENCE=1\n"
        "DATABASE=fixture.sigdb\n"
        "CRC32=CBF43926\n"
        "SIGNATURE=bde42589aa6f4a2bfbaef66ae83d4fc81505838e3a14795d3d74b4b1abc1b992c263783640ba594aadb07ec82772027151b3e448e6215d858d24ba9a26b53105\n";
    static const char manifest2[] =
        "AMIGUARD-SIGMANIFEST 1\n"
        "ALGORITHM=ED25519\n"
        "KEYID=ci-test-1\n"
        "SEQUENCE=2\n"
        "DATABASE=fixture.sigdb\n"
        "CRC32=CBF43926\n"
        "SIGNATURE=8106a25191168534b15ba3fd16f274d2cec95ac22b70f1d267422e6b22e222d083a850174bbff3378035f06a09dbe0719a56574c5942136a4a10f65e884d2b01\n";
    static const char bad_manifest[] =
        "AMIGUARD-SIGMANIFEST 1\n"
        "ALGORITHM=ED25519\n"
        "KEYID=ci-test-1\n"
        "SEQUENCE=3\n"
        "DATABASE=fixture.sigdb\n"
        "CRC32=CBF43926\n"
        "SIGNATURE=00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n";
    int ok = 1;

    if (!amiguard_ae_ed25519_auth_available()) {
        fprintf(stderr, "FAIL trusted CI key unavailable\n");
        return 1;
    }
    if (!write_text("build/m3_8b-1.manifest", manifest1) ||
        !write_text("build/m3_8b-2.manifest", manifest2) ||
        !write_text("build/m3_8b-bad.manifest", bad_manifest))
        return 1;

    ok = expect_auth("build/m3_8b-1.manifest", 1, "Ed25519 signature valid") && ok;
    if (ok)
        amiguard_ae_ed25519_auth_commit();
    if (amiguard_ae_ed25519_committed_sequence() != 1UL)
        ok = 0;

    ok = expect_auth("build/m3_8b-1.manifest", 0, "manifest sequence rollback rejected") && ok;
    ok = expect_auth("build/m3_8b-bad.manifest", 0, "Ed25519 signature verification failed") && ok;
    ok = expect_auth("build/m3_8b-2.manifest", 1, "Ed25519 signature valid") && ok;
    if (ok)
        amiguard_ae_ed25519_auth_commit();
    if (amiguard_ae_ed25519_committed_sequence() != 2UL)
        ok = 0;

    remove("build/m3_8b-1.manifest");
    remove("build/m3_8b-2.manifest");
    remove("build/m3_8b-bad.manifest");

    if (!ok)
        return 1;
    puts("M3.8b Ed25519 authentication qualification: PASS");
    return 0;
}
