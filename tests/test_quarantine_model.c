#include "quarantine_model.h"

#include <stdio.h>
#include <string.h>

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    const char *path = "build/m4_1_fixture.bin";
    FILE *fp;
    AmiGuardAEQuarantinePlan first;
    AmiGuardAEQuarantinePlan second;
    char detail[AMIGUARD_AE_QUARANTINE_DETAIL_MAX];

    if (amiguard_ae_quarantine_path_safe("SYS:", detail, sizeof(detail))) return fail("volume root accepted");
    if (amiguard_ae_quarantine_path_safe("/", detail, sizeof(detail))) return fail("filesystem root accepted");
    if (amiguard_ae_quarantine_path_safe("..", detail, sizeof(detail))) return fail("parent directory accepted");
    if (!amiguard_ae_quarantine_path_safe(path, detail, sizeof(detail))) return fail("fixture path rejected");

    fp = fopen(path, "wb");
    if (fp == 0) return fail("fixture create");
    if (fwrite("M4.1 quarantine fixture", 1, 23, fp) != 23) {
        fclose(fp);
        return fail("fixture write");
    }
    fclose(fp);

    if (!amiguard_ae_quarantine_plan(path, &first, detail, sizeof(detail))) return fail("first plan");
    if (!amiguard_ae_quarantine_plan(path, &second, detail, sizeof(detail))) return fail("second plan");
    if (strcmp(first.id, second.id) != 0) return fail("ID not deterministic");
    if (strcmp(first.source, path) != 0) return fail("source mismatch");
    if (first.source_size != 23UL) return fail("size mismatch");

    fp = fopen(path, "rb");
    if (fp == 0) return fail("planning modified/deleted source");
    fclose(fp);

    printf("M4.1 quarantine model qualification: PASS id=%s\n", first.id);
    return 0;
}
