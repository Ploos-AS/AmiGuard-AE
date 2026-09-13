#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"

static void trim_upper_token(const char *command, char *token, unsigned long size)
{
    unsigned long i = 0;
    if (size == 0) return;
    while (*command != '\0' && isspace((unsigned char)*command)) ++command;
    while (*command != '\0' && !isspace((unsigned char)*command) && i + 1 < size) {
        token[i++] = (char)toupper((unsigned char)*command++);
    }
    token[i] = '\0';
}

static void set_result(AmiGuardAERexxResult *out, long rc, const char *text)
{
    out->rc = rc;
    if (text == NULL) text = "";
    strncpy(out->result, text, AMIGUARD_AE_RESULT_MAX - 1);
    out->result[AMIGUARD_AE_RESULT_MAX - 1] = '\0';
}

void amiguard_ae_arexx_dispatch(const char *command, AmiGuardAERexxResult *out)
{
    char verb[32];
    char version[64];

    if (out == NULL) return;
    if (command == NULL) command = "";
    trim_upper_token(command, verb, sizeof(verb));

    if (strcmp(verb, "PING") == 0) {
        set_result(out, AMIGUARD_AE_RC_OK, "PONG");
    } else if (strcmp(verb, "VERSION") == 0) {
        sprintf(version, "%s %s", AMIGUARD_AE_NAME, amiguard_ae_version_string());
        set_result(out, AMIGUARD_AE_RC_OK, version);
    } else if (strcmp(verb, "STATUS") == 0) {
        set_result(out, AMIGUARD_AE_RC_OK, "READY M1 scanner=not-connected");
    } else if (strcmp(verb, "HELP") == 0) {
        set_result(out, AMIGUARD_AE_RC_OK, "PING VERSION STATUS HELP");
    } else if (verb[0] == '\0') {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR empty command");
    } else {
        set_result(out, AMIGUARD_AE_RC_ERROR, "ERROR unknown command");
    }
}
