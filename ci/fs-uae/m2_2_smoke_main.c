#include <stdio.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"

static int expect(const char *command, long expected_rc, const char *expected_result)
{
    AmiGuardAERexxResult result;
    amiguard_ae_arexx_dispatch(command, &result);
    if (result.rc != expected_rc) return 0;
    if (strcmp(result.result, expected_result) != 0) return 0;
    printf("SMOKE PASS %s => %s\n", command, result.result);
    return 1;
}

int main(void)
{
    int ok = 1;
    ok = expect("PING", AMIGUARD_AE_RC_OK, "PONG") && ok;
    ok = expect("STATUS", AMIGUARD_AE_RC_OK, "READY M2.2 scanner=connected") && ok;
    ok = expect("HELP", AMIGUARD_AE_RC_OK, "PING VERSION STATUS HELP SCANFILE") && ok;
    ok = expect("BOGUS", AMIGUARD_AE_RC_ERROR, "ERROR unknown command") && ok;
    if (!ok) return AMIGUARD_AE_RC_FAIL;
    puts("M2.2 AROS scanner bridge smoke: PASS");
    return AMIGUARD_AE_RC_OK;
}
