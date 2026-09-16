#include "audit_log.h"

#include <stdio.h>
#include <string.h>

static char audit_path_value[AMIGUARD_AE_AUDIT_PATH_MAX];

static void set_text(char *out, unsigned long size, const char *text)
{
    if (out == 0 || size == 0UL) return;
    if (text == 0) text = "";
    strncpy(out, text, (size_t)(size - 1UL));
    out[size - 1UL] = '\0';
}

static int field_safe(const char *text)
{
    const unsigned char *p;
    if (text == 0) return 0;
    p = (const unsigned char *)text;
    while (*p != 0U) {
        if (*p == '\n' || *p == '\r' || *p == '|' || *p < 32U) return 0;
        ++p;
    }
    return 1;
}

int amiguard_ae_audit_set_path(const char *path, char *detail,
                               unsigned long detail_size)
{
    unsigned long length;
    if (path == 0 || path[0] == '\0') {
        audit_path_value[0] = '\0';
        set_text(detail, detail_size, "audit path cleared; audit disabled");
        return 1;
    }
    length = (unsigned long)strlen(path);
    if (length >= sizeof(audit_path_value) || !field_safe(path) ||
        strcmp(path, ".") == 0 || strcmp(path, "..") == 0 ||
        strcmp(path, "/") == 0 || path[length - 1UL] == ':') {
        set_text(detail, detail_size, "unsafe audit path");
        return 0;
    }
    strcpy(audit_path_value, path);
    set_text(detail, detail_size, "audit path configured");
    return 1;
}

const char *amiguard_ae_audit_path(void)
{
    return audit_path_value;
}

int amiguard_ae_audit_available(void)
{
    return audit_path_value[0] != '\0';
}

int amiguard_ae_audit_append(const char *event, const char *status,
                             const char *subject, const char *detail,
                             char *error, unsigned long error_size)
{
    FILE *fp;
    int write_failed;
    int close_failed;

    if (!amiguard_ae_audit_available()) {
        set_text(error, error_size, "audit path unavailable");
        return 0;
    }
    if (!field_safe(event) || !field_safe(status) || !field_safe(subject) ||
        !field_safe(detail) || event[0] == '\0' || status[0] == '\0') {
        set_text(error, error_size, "invalid audit field");
        return 0;
    }
    fp = fopen(audit_path_value, "ab");
    if (fp == 0) {
        set_text(error, error_size, "audit open failed");
        return 0;
    }
    write_failed = fprintf(fp, "AMIGUARD-AUDIT|1|%s|%s|%s|%s\n",
                           event, status, subject, detail) < 0;
    close_failed = fclose(fp) != 0;
    if (write_failed || close_failed) {
        set_text(error, error_size, "audit append failed");
        return 0;
    }
    set_text(error, error_size, "audit record appended");
    return 1;
}
