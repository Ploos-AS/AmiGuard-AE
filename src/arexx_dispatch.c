#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"
#include "scanner_bridge.h"
#include "result_store.h"
#include "checksum.h"
#include "identify.h"

static void trim_upper_token(const char *command, char *token, unsigned long size)
{
    unsigned long i = 0;
    if (size == 0) return;
    while (*command != '\0' && isspace((unsigned char)*command)) ++command;
    while (*command != '\0' && !isspace((unsigned char)*command) && i + 1 < size)
        token[i++] = (char)toupper((unsigned char)*command++);
    token[i] = '\0';
}

static const char *command_argument(const char *command)
{
    while (*command != '\0' && isspace((unsigned char)*command)) ++command;
    while (*command != '\0' && !isspace((unsigned char)*command)) ++command;
    while (*command != '\0' && isspace((unsigned char)*command)) ++command;
    return command;
}

static void set_result(AmiGuardAERexxResult *out, long rc, const char *text)
{
    out->rc = rc;
    if (text == NULL) text = "";
    strncpy(out->result, text, AMIGUARD_AE_RESULT_MAX - 1);
    out->result[AMIGUARD_AE_RESULT_MAX - 1] = '\0';
}

static void dispatch_scanfile(const char *command, AmiGuardAERexxResult *out)
{
    const char *path = command_argument(command);
    AmiGuardAEScanResult scan;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*path == '\0') { set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR SCANFILE requires path"); return; }
    if (!amiguard_ae_scanner_scan_file(path, &scan)) { amiguard_ae_result_record(path, &scan); sprintf(text, "ERROR %s", scan.detail); set_result(out, AMIGUARD_AE_RC_ERROR, text); return; }
    amiguard_ae_result_record(path, &scan);
    if (scan.status == AMIGUARD_AE_SCAN_INFECTED) { sprintf(text, "INFECTED %s", scan.detail); set_result(out, AMIGUARD_AE_RC_WARN, text); }
    else if (scan.status == AMIGUARD_AE_SCAN_SUSPICIOUS) { sprintf(text, "SUSPICIOUS %s", scan.detail); set_result(out, AMIGUARD_AE_RC_WARN, text); }
    else if (scan.status == AMIGUARD_AE_SCAN_CLEAN) { sprintf(text, "CLEAN %s", scan.detail); set_result(out, AMIGUARD_AE_RC_OK, text); }
    else { sprintf(text, "ERROR %s", scan.detail); set_result(out, AMIGUARD_AE_RC_ERROR, text); }
}

static void dispatch_checksum(const char *command, AmiGuardAERexxResult *out)
{
    const char *path = command_argument(command);
    AmiGuardAEChecksumResult checksum;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*path == '\0') { set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR CHECKSUM requires path"); return; }
    if (!amiguard_ae_checksum_crc32(path, &checksum)) { sprintf(text, "ERROR %s", checksum.detail); set_result(out, AMIGUARD_AE_RC_ERROR, text); return; }
    sprintf(text, "CRC32 %s", checksum.checksum);
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_identify(const char *command, AmiGuardAERexxResult *out)
{
    const char *path = command_argument(command);
    AmiGuardAEIdentifyResult id;
    char text[AMIGUARD_AE_RESULT_MAX];
    if (*path == '\0') { set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR IDENTIFY requires path"); return; }
    if (!amiguard_ae_identify_file(path, &id)) { sprintf(text, "ERROR %s", id.detail); set_result(out, AMIGUARD_AE_RC_ERROR, text); return; }
    sprintf(text, "%s %s", id.type, id.detail);
    set_result(out, AMIGUARD_AE_RC_OK, text);
}

static void dispatch_result(const char *verb, AmiGuardAERexxResult *out)
{
    if (strcmp(verb, "RESULT.CLEAR") == 0) { amiguard_ae_result_clear(); set_result(out, AMIGUARD_AE_RC_OK, "OK"); return; }
    if (!amiguard_ae_result_valid()) { set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR no scan result"); return; }
    if (strcmp(verb, "RESULT.STATUS") == 0) set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_result_status());
    else if (strcmp(verb, "RESULT.PATH") == 0) set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_result_path());
    else if (strcmp(verb, "RESULT.DETAIL") == 0) set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_result_detail());
    else set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR unknown RESULT command");
}

void amiguard_ae_arexx_dispatch(const char *command, AmiGuardAERexxResult *out)
{
    char verb[32];
    char version[64];
    if (out == NULL) return;
    if (command == NULL) command = "";
    trim_upper_token(command, verb, sizeof(verb));
    if (strcmp(verb, "PING") == 0) set_result(out, AMIGUARD_AE_RC_OK, "PONG");
    else if (strcmp(verb, "VERSION") == 0) { sprintf(version, "%s %s", AMIGUARD_AE_NAME, amiguard_ae_version_string()); set_result(out, AMIGUARD_AE_RC_OK, version); }
    else if (strcmp(verb, "STATUS") == 0) set_result(out, AMIGUARD_AE_RC_OK, amiguard_ae_scanner_available() ? "READY M2.5 scanner=connected" : "READY M2.5 scanner=not-connected");
    else if (strcmp(verb, "HELP") == 0) set_result(out, AMIGUARD_AE_RC_OK, "PING VERSION STATUS HELP SCANFILE CHECKSUM IDENTIFY RESULT.STATUS RESULT.PATH RESULT.DETAIL RESULT.CLEAR");
    else if (strcmp(verb, "SCANFILE") == 0) dispatch_scanfile(command, out);
    else if (strcmp(verb, "CHECKSUM") == 0) dispatch_checksum(command, out);
    else if (strcmp(verb, "IDENTIFY") == 0) dispatch_identify(command, out);
    else if (strncmp(verb, "RESULT.", 7) == 0) dispatch_result(verb, out);
    else if (verb[0] == '\0') set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR empty command");
    else set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR unknown command");
}
