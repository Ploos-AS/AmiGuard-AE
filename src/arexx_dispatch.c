#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"
#include "scanner_bridge.h"
#include "signature_bridge.h"
#include "result_store.h"
#include "checksum.h"
#include "identify.h"

static void trim_upper_token(const char *command, char *token, unsigned long size)
{
    unsigned long i = 0;
    if (size == 0)
        return;
    while (*command != '\0' && isspace((unsigned char)*command))
        ++command;
    while (*command != '\0' && !isspace((unsigned char)*command) && i + 1 < size)
        token[i++] = (char)toupper((unsigned char)*command++);
    token[i] = '\0';
}

static const char *command_argument(const char *command)
{
    while (*command != '\0' && isspace((unsigned char)*command))
        ++command;
    while (*command != '\0' && !isspace((unsigned char)*command))
        ++command;
    while (*command != '\0' && isspace((unsigned char)*command))
        ++command;
    return command;
}

static void set_result(AmiGuardAERexxResult *out, long rc, const char *text)
{
    out->rc = rc;
    if (text == NULL)
        text = "";
    strncpy(out->result, text, AMIGUARD_AE_RESULT_MAX - 1);
    out->result[AMIGUARD_AE_RESULT_MAX - 1] = '\0';
}

static void dispatch_scan_target(const char *command, const char *verb, AmiGuardAERexxResult *out)
{
    const char *path = command_argument(command);
    AmiGuardAEScanResult scan;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*path == '\0') {
        sprintf(text, "ERROR %s requires path", verb);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    if (!amiguard_ae_scanner_scan_file(path, &scan)) {
        amiguard_ae_result_record(path, &scan);
        sprintf(text, "ERROR %s", scan.detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    amiguard_ae_result_record(path, &scan);
    if (scan.status == AMIGUARD_AE_SCAN_INFECTED) {
        sprintf(text, "INFECTED %s", scan.detail);
        set_result(out, AMIGUARD_AE_RC_WARN, text);
    } else if (scan.status == AMIGUARD_AE_SCAN_SUSPICIOUS) {
        sprintf(text, "SUSPICIOUS %s", scan.detail);
        set_result(out, AMIGUARD_AE_RC_WARN, text);
    } else if (scan.status == AMIGUARD_AE_SCAN_CLEAN) {
        sprintf(text, "CLEAN %s", scan.detail);
        set_result(out, AMIGUARD_AE_RC_OK, text);
    } else {
        sprintf(text, "ERROR %s", scan.detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
    }
}

static void dispatch_checksum(const char *command, AmiGuardAERexxResult *out)
{
    const char *path = command_argument(command);
    AmiGuardAEChecksumResult checksum;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*path == '\0') {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR CHECKSUM requires path");
        return;
    }
    if (!amiguard_ae_checksum_crc32(path, &checksum)) {
        sprintf(text, "ERROR %s", checksum.detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    sprintf(text, "CRC32 %s", checksum.checksum);
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_identify(const char *command, AmiGuardAERexxResult *out)
{
    const char *path = command_argument(command);
    AmiGuardAEIdentifyResult id;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*path == '\0') {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR IDENTIFY requires path");
        return;
    }
    if (!amiguard_ae_identify_file(path, &id)) {
        sprintf(text, "ERROR %s", id.detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    sprintf(text, "%s %s", id.type, id.detail);
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_signature_count(AmiGuardAERexxResult *out)
{
    char text[AMIGUARD_AE_RESULT_MAX];
    if (!amiguard_ae_signature_available()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR signature backend unavailable");
        return;
    }
    sprintf(text, "%lu", amiguard_ae_signature_count());
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_signature_status(const char *command, AmiGuardAERexxResult *out)
{
    const char *arg = command_argument(command);
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*arg != '\0') {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR SIGNATURE.STATUS takes no arguments");
        return;
    }
    if (!amiguard_ae_signature_available()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR signature backend unavailable");
        return;
    }
    sprintf(text,
            "READY COUNT=%lu UPDATE=%s VERIFY=CRC32 AUTH=%s",
            amiguard_ae_signature_count(),
            amiguard_ae_signature_update_available() ? "AVAILABLE" : "UNAVAILABLE",
            amiguard_ae_signature_auth_available() ? "AVAILABLE" : "UNAVAILABLE");
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_signature_info(const char *command, AmiGuardAERexxResult *out)
{
    const char *arg = command_argument(command);
    char *end = 0;
    unsigned long index;
    AmiGuardAESignatureInfo info;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (!amiguard_ae_signature_available()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR signature backend unavailable");
        return;
    }
    if (*arg == '\0') {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR SIGNATURE.INFO requires index");
        return;
    }
    index = strtoul(arg, &end, 10);
    if (end == arg) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR invalid signature index");
        return;
    }
    while (*end != '\0' && isspace((unsigned char)*end))
        ++end;
    if (*end != '\0' || index >= amiguard_ae_signature_count()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR invalid signature index");
        return;
    }
    if (!amiguard_ae_signature_info(index, &info)) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR signature metadata unavailable");
        return;
    }
    sprintf(text, "INDEX=%lu TYPE=%s OFFSET=%lu LENGTH=%lu TEST_ONLY=%d NAME=%s", index, info.type, info.offset, info.length, info.test_only, info.name);
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static int valid_crc32_text(const char *text)
{
    unsigned int i;
    if (text == 0 || strlen(text) != 8U)
        return 0;
    for (i = 0U; i < 8U; ++i) {
        if (!isxdigit((unsigned char)text[i]))
            return 0;
    }
    return 1;
}

static int next_token(const char **cursor, char *out, unsigned long out_size)
{
    const char *p = *cursor;
    const char *start;
    unsigned long n;
    while (*p != '\0' && isspace((unsigned char)*p))
        ++p;
    start = p;
    while (*p != '\0' && !isspace((unsigned char)*p))
        ++p;
    n = (unsigned long)(p - start);
    if (n == 0UL || n >= out_size)
        return 0;
    memcpy(out, start, (size_t)n);
    out[n] = '\0';
    *cursor = p;
    return 1;
}

static void dispatch_signature_update(const char *command, AmiGuardAERexxResult *out)
{
    const char *cursor = command_argument(command);
    char path[256];
    char expected[9];
    char manifest[256];
    AmiGuardAEChecksumResult checksum;
    char detail[AMIGUARD_AE_SIGNATURE_UPDATE_DETAIL_MAX];
    char text[AMIGUARD_AE_RESULT_MAX];
    unsigned int i;

    if (!next_token(&cursor, path, sizeof(path))) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR SIGNATURE.UPDATE requires path CRC32 manifest");
        return;
    }
    if (!next_token(&cursor, expected, sizeof(expected))) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR SIGNATURE.UPDATE requires CRC32 and manifest");
        return;
    }
    if (!next_token(&cursor, manifest, sizeof(manifest))) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR SIGNATURE.UPDATE requires manifest");
        return;
    }
    while (*cursor != '\0' && isspace((unsigned char)*cursor))
        ++cursor;
    if (*cursor != '\0' || !valid_crc32_text(expected)) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR invalid SIGNATURE.UPDATE arguments");
        return;
    }
    for (i = 0U; i < 8U; ++i)
        expected[i] = (char)toupper((unsigned char)expected[i]);

    if (!amiguard_ae_signature_update_available()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR signature updater unavailable");
        return;
    }
    if (!amiguard_ae_checksum_crc32(path, &checksum)) {
        sprintf(text, "ERROR %s", checksum.detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    if (strcmp(checksum.checksum, expected) != 0) {
        sprintf(text, "ERROR checksum mismatch expected=%s actual=%s", expected, checksum.checksum);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    if (!amiguard_ae_signature_auth_available()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR signature authenticator unavailable");
        return;
    }
    if (!amiguard_ae_signature_authenticate(manifest, path, expected, detail, sizeof(detail))) {
        if (detail[0] == '\0')
            strcpy(detail, "authentication failed");
        sprintf(text, "ERROR %s", detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    if (!amiguard_ae_signature_update(path, detail, sizeof(detail))) {
        if (detail[0] == '\0')
            strcpy(detail, "update failed");
        sprintf(text, "ERROR %s", detail);
        set_result(out, AMIGUARD_AE_RC_ERROR, text);
        return;
    }
    amiguard_ae_signature_auth_commit();
    if (detail[0] == '\0')
        strcpy(detail, "updated");
    sprintf(text, "UPDATED VERIFIED=CRC32 AUTH=ED25519 %s", detail);
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_result(const char *verb, AmiGuardAERexxResult *out)
{
    if (strcmp(verb, "RESULT.CLEAR") == 0) {
        amiguard_ae_result_clear();
        set_result(out, AMIGUARD_AE_RC_OK, "OK");
        return;
    }
    if (!amiguard_ae_result_valid()) {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR no scan result");
        return;
    }
    if (strcmp(verb, "RESULT.STATUS") == 0)
        set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_result_status());
    else if (strcmp(verb, "RESULT.PATH") == 0)
        set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_result_path());
    else if (strcmp(verb, "RESULT.DETAIL") == 0)
        set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_result_detail());
    else
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR unknown RESULT command");
}

void amiguard_ae_arexx_dispatch(const char *command, AmiGuardAERexxResult *out)
{
    char verb[32];
    char version[64];
    if (out == NULL)
        return;
    if (command == NULL)
        command = "";
    trim_upper_token(command, verb, sizeof(verb));
    if (strcmp(verb, "PING") == 0)
        set_result(out, AMIGUARD_AE_RC_OK, "PONG");
    else if (strcmp(verb, "VERSION") == 0) {
        sprintf(version, "%s %s", AMIGUARD_AE_NAME, amiguard_ae_version_string());
        set_result(out, AMIGUARD_AE_RC_OK, version);
    } else if (strcmp(verb, "STATUS") == 0)
        set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_scanner_available() ? "READY M3.8b scanner=connected" : "READY M3.8b scanner=not-connected");
    else if (strcmp(verb, "HELP") == 0)
        set_result(out, AMIGUARD_AE_RC_OK, "PING VERSION STATUS HELP SCAN SCANFILE CHECKSUM IDENTIFY SIGNATURE.COUNT SIGNATURE.STATUS SIGNATURE.INFO SIGNATURE.UPDATE RESULT.STATUS RESULT.PATH RESULT.DETAIL RESULT.CLEAR");
    else if (strcmp(verb, "SCAN") == 0)
        dispatch_scan_target(command, "SCAN", out);
    else if (strcmp(verb, "SCANFILE") == 0)
        dispatch_scan_target(command, "SCANFILE", out);
    else if (strcmp(verb, "CHECKSUM") == 0)
        dispatch_checksum(command, out);
    else if (strcmp(verb, "IDENTIFY") == 0)
        dispatch_identify(command, out);
    else if (strcmp(verb, "SIGNATURE.COUNT") == 0)
        dispatch_signature_count(out);
    else if (strcmp(verb, "SIGNATURE.STATUS") == 0)
        dispatch_signature_status(command, out);
    else if (strcmp(verb, "SIGNATURE.INFO") == 0)
        dispatch_signature_info(command, out);
    else if (strcmp(verb, "SIGNATURE.UPDATE") == 0)
        dispatch_signature_update(command, out);
    else if (strncmp(verb, "RESULT.", 7) == 0)
        dispatch_result(verb, out);
    else if (verb[0] == '\0')
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR empty command");
    else
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR unknown command");
}
