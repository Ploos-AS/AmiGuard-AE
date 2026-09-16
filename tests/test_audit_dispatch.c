#include <stdio.h>
#include <string.h>
#include "arexx_dispatch.h"
#include "scanner_bridge.h"
#include "signature_bridge.h"
#include "quarantine_model.h"
#include "audit_log.h"

static int fake_scan(const char *path, AmiGuardAEScanResult *out)
{
    (void)path;
    out->status = AMIGUARD_AE_SCAN_CLEAN;
    strcpy(out->detail, "known-clean");
    return 1;
}

static unsigned long fake_count(void) { return 1UL; }
static int fake_update(const char *path, char *detail, unsigned long size)
{
    (void)path;
    if (size < 8UL) return 0;
    strcpy(detail, "updated");
    return 1;
}
static int fake_auth(const char *manifest, const char *database,
                     const char *crc32, char *detail, unsigned long size)
{
    (void)manifest; (void)database; (void)crc32;
    if (size < 3UL) return 0;
    strcpy(detail, "ok");
    return 1;
}

static int write_file(const char *path, const char *text)
{
    FILE *fp = fopen(path, "wb");
    size_t n = strlen(text);
    if (fp == 0) return 0;
    if (fwrite(text, 1U, n, fp) != n) { fclose(fp); return 0; }
    fclose(fp);
    return 1;
}

static int contains(const char *path, const char *needle)
{
    FILE *fp = fopen(path, "rb");
    char line[768];
    if (fp == 0) return 0;
    while (fgets(line, sizeof(line), fp) != 0) {
        if (strstr(line, needle) != 0) { fclose(fp); return 1; }
    }
    fclose(fp);
    return 0;
}

int main(void)
{
    const char *audit = "build/m4_5_dispatch_audit.log";
    const char *source = "build/m4_5_dispatch_quarantine.bin";
    const char *sigdb = "build/m4_5_dispatch.sigdb";
    AmiGuardAEQuarantinePlan plan;
    AmiGuardAERexxResult out;
    char detail[AMIGUARD_AE_QUARANTINE_DETAIL_MAX];
    char command[384];
    char qobject[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char qmeta[AMIGUARD_AE_QUARANTINE_PATH_MAX];

    remove(audit);
    if (!amiguard_ae_audit_set_path(audit, detail, sizeof(detail))) return 1;
    amiguard_ae_scanner_set_provider(fake_scan);
    amiguard_ae_signature_set_count_provider(fake_count);
    amiguard_ae_signature_set_update_provider(fake_update);
    amiguard_ae_signature_set_auth_provider(fake_auth);

    amiguard_ae_arexx_dispatch("SCAN clean.bin", &out);
    if (out.rc != 0) return 1;

    if (!write_file(sigdb, "123456789")) return 1;
    sprintf(command, "SIGNATURE.UPDATE %s CBF43926 build/manifest", sigdb);
    amiguard_ae_arexx_dispatch(command, &out);
    if (out.rc != 0) return 1;

    if (!write_file(source, "M4.5 audit dispatcher quarantine")) return 1;
    if (!amiguard_ae_quarantine_plan(source, &plan, detail, sizeof(detail))) return 1;
    sprintf(qobject, "build/%s.qtn", plan.id);
    sprintf(qmeta, "build/%s.meta", plan.id);
    remove(qobject); remove(qmeta);
    if (!amiguard_ae_quarantine_set_directory("build", detail, sizeof(detail))) return 1;
    sprintf(command, "QUARANTINE %s", source);
    amiguard_ae_arexx_dispatch(command, &out);
    if (out.rc != 0) return 1;
    sprintf(command, "RESTORE %s", plan.id);
    amiguard_ae_arexx_dispatch(command, &out);
    if (out.rc != 0) return 1;

    if (!contains(audit, "|SCAN|OK|clean.bin|CLEAN known-clean")) return 1;
    if (!contains(audit, "|SIGNATURE.UPDATE|OK|build/m4_5_dispatch.sigdb CBF43926 build/manifest|UPDATED VERIFIED=CRC32 AUTH=CUSTOM updated")) return 1;
    if (!contains(audit, "|QUARANTINE|OK|build/m4_5_dispatch_quarantine.bin|QUARANTINED ID=")) return 1;
    if (!contains(audit, "|RESTORE|OK|Q")) return 1;

    remove(source); remove(sigdb); remove(qobject); remove(qmeta); remove(audit);
    puts("M4.5 dispatcher audit integration: PASS");
    return 0;
}
