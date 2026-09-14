#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "signature_auth_ed25519.h"
#include "signature_manifest.h"

#if defined(AMIGUARD_AE_WITH_ED25519)
#include "ed25519.h"
#endif

#define AUTH_PAYLOAD_MAX 768

static unsigned long committed_sequence = 0UL;
static unsigned long pending_sequence = 0UL;

static void set_detail(char *detail, unsigned long size, const char *text)
{
    if (detail == 0 || size == 0UL)
        return;
    if (text == 0)
        text = "";
    strncpy(detail, text, (size_t)(size - 1UL));
    detail[size - 1UL] = '\0';
}

#if defined(AMIGUARD_AE_WITH_ED25519) && \
    defined(AMIGUARD_AE_TRUSTED_KEY_ID) && \
    defined(AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX)
static int hex_value(int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    ch = toupper((unsigned char)ch);
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    return -1;
}

static int decode_hex(const char *text, unsigned char *out, unsigned long out_size)
{
    unsigned long i;
    int hi;
    int lo;
    if (text == 0 || strlen(text) != (size_t)(out_size * 2UL))
        return 0;
    for (i = 0UL; i < out_size; ++i) {
        hi = hex_value((unsigned char)text[i * 2UL]);
        lo = hex_value((unsigned char)text[i * 2UL + 1UL]);
        if (hi < 0 || lo < 0)
            return 0;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 1;
}

static const char *base_name(const char *path)
{
    const char *p;
    const char *base = path;
    if (path == 0)
        return "";
    for (p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\' || *p == ':')
            base = p + 1;
    }
    return base;
}
#endif

int amiguard_ae_ed25519_auth_available(void)
{
#if defined(AMIGUARD_AE_WITH_ED25519) && \
    defined(AMIGUARD_AE_TRUSTED_KEY_ID) && \
    defined(AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX)
    return 1;
#else
    return 0;
#endif
}

int amiguard_ae_ed25519_authenticate(const char *manifest_path,
                                     const char *database_path,
                                     const char *crc32,
                                     char *detail,
                                     unsigned long detail_size)
{
#if defined(AMIGUARD_AE_WITH_ED25519) && \
    defined(AMIGUARD_AE_TRUSTED_KEY_ID) && \
    defined(AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX)
    AmiGuardAESignatureManifest manifest;
    unsigned char signature[64];
    unsigned char public_key[32];
    char payload[AUTH_PAYLOAD_MAX];
    int payload_len;

    pending_sequence = 0UL;
    if (!amiguard_ae_signature_manifest_load(manifest_path,
                                             &manifest,
                                             detail,
                                             detail_size))
        return 0;
    if (strcmp(manifest.key_id, AMIGUARD_AE_TRUSTED_KEY_ID) != 0) {
        set_detail(detail, detail_size, "untrusted manifest key id");
        return 0;
    }
    if (strcmp(manifest.database, base_name(database_path)) != 0) {
        set_detail(detail, detail_size, "manifest database mismatch");
        return 0;
    }
    if (crc32 == 0 || strcmp(manifest.crc32, crc32) != 0) {
        set_detail(detail, detail_size, "manifest CRC32 mismatch");
        return 0;
    }
    if (manifest.sequence <= committed_sequence) {
        set_detail(detail, detail_size, "manifest sequence rollback rejected");
        return 0;
    }
    if (!decode_hex(manifest.signature_hex, signature, 64UL) ||
        !decode_hex(AMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX, public_key, 32UL)) {
        set_detail(detail, detail_size, "invalid Ed25519 key or signature encoding");
        return 0;
    }

    payload_len = sprintf(payload,
                          "AMIGUARD-SIGMANIFEST 1\n"
                          "ALGORITHM=%s\n"
                          "KEYID=%s\n"
                          "SEQUENCE=%lu\n"
                          "DATABASE=%s\n"
                          "CRC32=%s\n",
                          manifest.algorithm,
                          manifest.key_id,
                          manifest.sequence,
                          manifest.database,
                          manifest.crc32);
    if (payload_len <= 0 || (unsigned long)payload_len >= sizeof(payload)) {
        set_detail(detail, detail_size, "manifest payload too large");
        return 0;
    }
    if (!ed25519_verify(signature,
                        (const unsigned char *)payload,
                        (size_t)payload_len,
                        public_key)) {
        set_detail(detail, detail_size, "Ed25519 signature verification failed");
        return 0;
    }

    pending_sequence = manifest.sequence;
    set_detail(detail, detail_size, "Ed25519 signature valid");
    return 1;
#else
    (void)manifest_path;
    (void)database_path;
    (void)crc32;
    set_detail(detail, detail_size, "trusted Ed25519 key not provisioned");
    return 0;
#endif
}

void amiguard_ae_ed25519_auth_commit(void)
{
    if (pending_sequence > committed_sequence)
        committed_sequence = pending_sequence;
    pending_sequence = 0UL;
}

unsigned long amiguard_ae_ed25519_committed_sequence(void)
{
    return committed_sequence;
}
