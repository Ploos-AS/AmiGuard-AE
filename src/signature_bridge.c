#include <string.h>

#include "signature_bridge.h"
#include "signature_auth_ed25519.h"

static AmiGuardAESignatureCountProvider count_provider = 0;
static AmiGuardAESignatureInfoProvider info_provider = 0;
static AmiGuardAESignatureUpdateProvider update_provider = 0;
static AmiGuardAESignatureAuthProvider auth_provider = 0;

#if defined(AMIGUARD_AE_WITH_AMIGUARD)
#include "file_signatures.h"
#include "signatures.h"

static void copy_text(char *dst, unsigned long size, const char *src)
{
    if (size == 0UL)
        return;
    if (src == 0)
        src = "";
    strncpy(dst, src, (size_t)(size - 1UL));
    dst[size - 1UL] = '\0';
}

static unsigned long builtin_signature_count(void)
{
    unsigned long boot_count = 0UL;
    (void)amiguard_signatures(&boot_count);
    return boot_count + amiguard_file_signature_count();
}

static int builtin_signature_info(unsigned long index, AmiGuardAESignatureInfo *out)
{
    const struct amiguard_signature *boot;
    const struct amiguard_file_signature *file;
    unsigned long boot_count = 0UL;
    unsigned long file_count = 0UL;

    if (out == 0)
        return 0;

    boot = amiguard_signatures(&boot_count);
    if (index < boot_count) {
        copy_text(out->type, AMIGUARD_AE_SIGNATURE_TYPE_MAX, "BOOTBLOCK");
        copy_text(out->name, AMIGUARD_AE_SIGNATURE_NAME_MAX, boot[index].name);
        out->offset = (unsigned long)boot[index].offset;
        out->length = (unsigned long)boot[index].length;
        out->test_only = 0;
        return 1;
    }

    file = amiguard_file_signatures(&file_count);
    index -= boot_count;
    if (index >= file_count)
        return 0;

    copy_text(out->type, AMIGUARD_AE_SIGNATURE_TYPE_MAX, "FILE");
    copy_text(out->name, AMIGUARD_AE_SIGNATURE_NAME_MAX, file[index].name);
    out->offset = file[index].offset;
    out->length = (unsigned long)file[index].length;
    out->test_only = file[index].test_only;
    return 1;
}

static int builtin_signature_update(const char *path,
                                    char *detail,
                                    unsigned long detail_size)
{
    return amiguard_file_signature_load_database(path, detail, detail_size);
}
#endif

void amiguard_ae_signature_set_count_provider(AmiGuardAESignatureCountProvider provider)
{
    count_provider = provider;
}

void amiguard_ae_signature_set_info_provider(AmiGuardAESignatureInfoProvider provider)
{
    info_provider = provider;
}

void amiguard_ae_signature_set_update_provider(AmiGuardAESignatureUpdateProvider provider)
{
    update_provider = provider;
}

void amiguard_ae_signature_set_auth_provider(AmiGuardAESignatureAuthProvider provider)
{
    auth_provider = provider;
}

int amiguard_ae_signature_available(void)
{
    if (count_provider != 0)
        return 1;
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return 1;
#else
    return 0;
#endif
}

int amiguard_ae_signature_update_available(void)
{
    if (update_provider != 0)
        return 1;
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return 1;
#else
    return 0;
#endif
}

int amiguard_ae_signature_auth_available(void)
{
    if (auth_provider != 0)
        return 1;
    return amiguard_ae_ed25519_auth_available();
}

unsigned long amiguard_ae_signature_count(void)
{
    if (count_provider != 0)
        return count_provider();
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return builtin_signature_count();
#else
    return 0UL;
#endif
}

int amiguard_ae_signature_info(unsigned long index, AmiGuardAESignatureInfo *out)
{
    if (info_provider != 0)
        return info_provider(index, out);
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return builtin_signature_info(index, out);
#else
    (void)index;
    (void)out;
    return 0;
#endif
}

int amiguard_ae_signature_authenticate(const char *manifest_path,
                                       const char *database_path,
                                       const char *crc32,
                                       char *detail,
                                       unsigned long detail_size)
{
    if (detail != 0 && detail_size != 0UL)
        detail[0] = '\0';
    if (auth_provider != 0)
        return auth_provider(manifest_path, database_path, crc32, detail, detail_size);
    return amiguard_ae_ed25519_authenticate(manifest_path,
                                            database_path,
                                            crc32,
                                            detail,
                                            detail_size);
}

void amiguard_ae_signature_auth_commit(void)
{
    if (auth_provider == 0)
        amiguard_ae_ed25519_auth_commit();
}

unsigned long amiguard_ae_signature_auth_sequence(void)
{
    if (auth_provider != 0)
        return 0UL;
    return amiguard_ae_ed25519_committed_sequence();
}

int amiguard_ae_signature_update(const char *path,
                                 char *detail,
                                 unsigned long detail_size)
{
    if (detail != 0 && detail_size != 0UL)
        detail[0] = '\0';

    if (update_provider != 0)
        return update_provider(path, detail, detail_size);
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return builtin_signature_update(path, detail, detail_size);
#else
    (void)path;
    return 0;
#endif
}
