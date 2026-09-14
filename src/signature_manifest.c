#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "signature_manifest.h"

#define LINE_MAX 512

static void set_detail(char *detail, unsigned long size, const char *text)
{
    if (detail == 0 || size == 0UL)
        return;
    if (text == 0)
        text = "";
    strncpy(detail, text, (size_t)(size - 1UL));
    detail[size - 1UL] = '\0';
}

static void trim_eol(char *text)
{
    char *p = strpbrk(text, "\r\n");
    if (p != 0)
        *p = '\0';
}

static int is_hex_string(const char *text, unsigned long length)
{
    unsigned long i;
    if (strlen(text) != length)
        return 0;
    for (i = 0UL; i < length; ++i) {
        if (!isxdigit((unsigned char)text[i]))
            return 0;
    }
    return 1;
}

static int copy_value(char *dst, unsigned long size, const char *src)
{
    unsigned long n;
    if (dst == 0 || src == 0 || size == 0UL)
        return 0;
    n = (unsigned long)strlen(src);
    if (n == 0UL || n >= size)
        return 0;
    strcpy(dst, src);
    return 1;
}

int amiguard_ae_signature_manifest_load(const char *path,
                                        AmiGuardAESignatureManifest *out,
                                        char *detail,
                                        unsigned long detail_size)
{
    FILE *fp;
    char line[LINE_MAX];
    unsigned long field = 0UL;
    char *value;
    char *end;

    if (out == 0) {
        set_detail(detail, detail_size, "manifest output required");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    set_detail(detail, detail_size, "");
    if (path == 0 || path[0] == '\0') {
        set_detail(detail, detail_size, "manifest path required");
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == 0) {
        set_detail(detail, detail_size, "cannot open manifest");
        return 0;
    }

    while (fgets(line, sizeof(line), fp) != 0) {
        size_t len = strlen(line);
        if (len == sizeof(line) - 1U && line[len - 1U] != '\n') {
            fclose(fp);
            set_detail(detail, detail_size, "manifest line too long");
            return 0;
        }
        trim_eol(line);
        if (line[0] == '\0' || line[0] == '#')
            continue;

        if (field == 0UL) {
            if (strcmp(line, "AMIGUARD-SIGMANIFEST 1") != 0) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest header");
                return 0;
            }
        } else if (field == 1UL) {
            if (strncmp(line, "ALGORITHM=", 10U) != 0 ||
                !copy_value(out->algorithm, sizeof(out->algorithm), line + 10U) ||
                strcmp(out->algorithm, "ED25519") != 0) {
                fclose(fp);
                set_detail(detail, detail_size, "unsupported manifest algorithm");
                return 0;
            }
        } else if (field == 2UL) {
            if (strncmp(line, "KEYID=", 6U) != 0 ||
                !copy_value(out->key_id, sizeof(out->key_id), line + 6U)) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest key id");
                return 0;
            }
        } else if (field == 3UL) {
            if (strncmp(line, "SEQUENCE=", 9U) != 0) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest sequence");
                return 0;
            }
            value = line + 9U;
            out->sequence = strtoul(value, &end, 10);
            if (end == value || *end != '\0' || out->sequence == 0UL) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest sequence");
                return 0;
            }
        } else if (field == 4UL) {
            if (strncmp(line, "DATABASE=", 9U) != 0 ||
                !copy_value(out->database, sizeof(out->database), line + 9U)) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest database");
                return 0;
            }
        } else if (field == 5UL) {
            if (strncmp(line, "CRC32=", 6U) != 0 ||
                !copy_value(out->crc32, sizeof(out->crc32), line + 6U) ||
                !is_hex_string(out->crc32, 8UL)) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest CRC32");
                return 0;
            }
        } else if (field == 6UL) {
            if (strncmp(line, "SIGNATURE=", 10U) != 0 ||
                !copy_value(out->signature_hex, sizeof(out->signature_hex), line + 10U) ||
                !is_hex_string(out->signature_hex, 128UL)) {
                fclose(fp);
                set_detail(detail, detail_size, "invalid manifest signature");
                return 0;
            }
        } else {
            fclose(fp);
            set_detail(detail, detail_size, "unexpected manifest field");
            return 0;
        }
        ++field;
    }

    if (ferror(fp)) {
        fclose(fp);
        set_detail(detail, detail_size, "error reading manifest");
        return 0;
    }
    fclose(fp);

    if (field != 7UL) {
        set_detail(detail, detail_size, "manifest incomplete");
        return 0;
    }

    set_detail(detail, detail_size, "signed manifest syntax valid");
    return 1;
}
