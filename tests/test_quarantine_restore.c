#include "quarantine_model.h"

#include <stdio.h>
#include <string.h>

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

static int exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == 0) return 0;
    fclose(fp);
    return 1;
}

static int write_bad_metadata(const char *path,
                              const AmiGuardAEQuarantinePlan *plan,
                              const char *crc,
                              const char *size)
{
    FILE *fp = fopen(path, "wb");
    if (fp == 0) return 0;
    if (fprintf(fp, "AMIGUARD-QUARANTINE 1\nID=%s\nSOURCE=%s\nCRC32=%s\nSIZE=%s\n",
                plan->id, plan->source, crc, size) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

int main(void)
{
    const char *source = "build/m4_4_source.bin";
    AmiGuardAEQuarantinePlan plan;
    char detail[AMIGUARD_AE_QUARANTINE_DETAIL_MAX];
    char stored[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char restored[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char metadata[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    FILE *fp;

    remove(source);
    if (!amiguard_ae_quarantine_set_directory("build", detail, sizeof(detail))) return fail(detail);

    if (amiguard_ae_quarantine_restore("Q123", restored, sizeof(restored), detail, sizeof(detail))) return fail("short quarantine id accepted");
    if (strcmp(detail, "invalid quarantine id") != 0) return fail("short id detail");
    if (amiguard_ae_quarantine_restore("Q123456789ABCDEG", restored, sizeof(restored), detail, sizeof(detail))) return fail("non-hex quarantine id accepted");
    if (strcmp(detail, "invalid quarantine id") != 0) return fail("non-hex id detail");

    fp = fopen(source, "wb");
    if (fp == 0) return fail("source create");
    if (fwrite("M4.4 verified restore", 1, 21, fp) != 21) { fclose(fp); return fail("source write"); }
    fclose(fp);
    if (!amiguard_ae_quarantine_plan(source, &plan, detail, sizeof(detail))) return fail(detail);
    sprintf(metadata, "build/%s.meta", plan.id);
    remove(metadata);
    if (!amiguard_ae_quarantine_store(&plan, "build", stored, sizeof(stored), detail, sizeof(detail))) return fail(detail);
    if (exists(source)) return fail("source survived quarantine");
    if (!amiguard_ae_quarantine_restore(plan.id, restored, sizeof(restored), detail, sizeof(detail))) return fail(detail);
    if (strcmp(restored, source) != 0 || !exists(source)) return fail("restore destination missing");
    if (!exists(stored) || !exists(metadata)) return fail("restore must retain quarantine evidence");

    /* A restore must never silently overwrite an existing destination. */
    if (amiguard_ae_quarantine_restore(plan.id, restored, sizeof(restored), detail, sizeof(detail))) return fail("existing destination overwritten");
    if (strcmp(detail, "restore destination exists; refusing overwrite") != 0) return fail("overwrite refusal detail");

    remove(source);
    /* Metadata numeric fields must reject malformed/trailing input. */
    if (!write_bad_metadata(metadata, &plan, "1234567G", "21")) return fail("bad CRC metadata write");
    if (amiguard_ae_quarantine_restore(plan.id, restored, sizeof(restored), detail, sizeof(detail))) return fail("malformed CRC metadata accepted");
    if (strcmp(detail, "invalid quarantine metadata") != 0) return fail("malformed CRC detail");
    if (!write_bad_metadata(metadata, &plan, "12345678", "21junk")) return fail("bad SIZE metadata write");
    if (amiguard_ae_quarantine_restore(plan.id, restored, sizeof(restored), detail, sizeof(detail))) return fail("malformed SIZE metadata accepted");
    if (strcmp(detail, "invalid quarantine metadata") != 0) return fail("malformed SIZE detail");

    /* Tampered quarantine content must fail verification. */
    if (!write_bad_metadata(metadata, &plan, "00000000", "8")) return fail("metadata rewrite");
    fp = fopen(stored, "wb");
    if (fp == 0) return fail("tamper open");
    if (fwrite("tampered", 1, 8, fp) != 8) { fclose(fp); return fail("tamper write"); }
    fclose(fp);
    if (amiguard_ae_quarantine_restore(plan.id, restored, sizeof(restored), detail, sizeof(detail))) return fail("tampered object restored");
    if (exists(source)) return fail("tampered restore created destination");

    remove(stored);
    remove(metadata);
    amiguard_ae_quarantine_set_directory(0, detail, sizeof(detail));
    puts("M4 hardening verified restore qualification: PASS");
    return 0;
}
