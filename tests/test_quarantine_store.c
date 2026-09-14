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

int main(void)
{
    const char *source = "build/m4_2_source.bin";
    const char *destination = "build/Q9D2C4F3A0000017.qtn";
    const char *metadata = "build/quarantine.meta";
    FILE *fp;
    AmiGuardAEQuarantinePlan plan;
    char detail[AMIGUARD_AE_QUARANTINE_DETAIL_MAX];
    char stored[AMIGUARD_AE_QUARANTINE_PATH_MAX];

    remove(destination);
    remove(metadata);
    remove("build/.amiguard-stage");
    remove("build/.quarantine.meta.stage");

    fp = fopen(source, "wb");
    if (fp == 0) return fail("source create");
    if (fwrite("M4.2 transactional quarantine", 1, 29, fp) != 29) {
        fclose(fp);
        return fail("source write");
    }
    fclose(fp);

    if (!amiguard_ae_quarantine_plan(source, &plan, detail, sizeof(detail)))
        return fail("plan");
    if (!amiguard_ae_quarantine_store(&plan, "build", stored, sizeof(stored), detail, sizeof(detail)))
        return fail(detail);
    if (exists(source)) return fail("source still exists after successful commit");
    if (!exists(stored)) return fail("quarantine object missing");
    if (!exists(metadata)) return fail("metadata missing");

    /* Existing destinations must never be overwritten. */
    fp = fopen(source, "wb");
    if (fp == 0) return fail("second source create");
    if (fwrite("replacement", 1, 11, fp) != 11) {
        fclose(fp);
        return fail("second source write");
    }
    fclose(fp);
    if (!amiguard_ae_quarantine_plan(source, &plan, detail, sizeof(detail)))
        return fail("second plan");
    if (amiguard_ae_quarantine_store(&plan, "build", stored, sizeof(stored), detail, sizeof(detail)))
        return fail("existing destination accepted");
    if (!exists(source)) return fail("source destroyed on rejected overwrite");

    remove(source);
    remove(destination);
    remove(metadata);
    remove("build/.amiguard-stage");
    remove("build/.quarantine.meta.stage");
    printf("M4.2 transactional quarantine qualification: PASS\n");
    return 0;
}
