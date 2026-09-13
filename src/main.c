#include <stdio.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"

#if defined(__AMIGA__) && !defined(AMIGUARD_AE_AROS_SMOKE)
#include "arexx_amiga.h"
#include <exec/tasks.h>
#include <proto/exec.h>
#endif

#if defined(__AMIGA__) && defined(AMIGUARD_AE_AROS_SMOKE)
static int smoke_expect(const char *command, long expected_rc, const char *expected_result)
{
    AmiGuardAERexxResult result;

    amiguard_ae_arexx_dispatch(command, &result);
    if (result.rc != expected_rc) {
        printf("SMOKE FAIL command=%s rc=%ld expected=%ld\n",
               command, result.rc, expected_rc);
        return 0;
    }
    if (expected_result != NULL && strcmp(result.result, expected_result) != 0) {
        printf("SMOKE FAIL command=%s result=%s expected=%s\n",
               command, result.result, expected_result);
        return 0;
    }
    printf("SMOKE PASS command=%s rc=%ld result=%s\n",
           command, result.rc, result.result);
    return 1;
}
#endif

int main(void)
{
#if defined(__AMIGA__) && defined(AMIGUARD_AE_AROS_SMOKE)
    int ok = 1;

    printf("%s %s - AROS native dispatcher smoke\n",
           AMIGUARD_AE_NAME, amiguard_ae_version_string());
    ok = smoke_expect("PING", AMIGUARD_AE_RC_OK, "PONG") && ok;
    ok = smoke_expect("VERSION", AMIGUARD_AE_RC_OK, "AmiGuard AE 0.1.0-m1") && ok;
    ok = smoke_expect("STATUS", AMIGUARD_AE_RC_OK, "READY M1 scanner=not-connected") && ok;
    ok = smoke_expect("HELP", AMIGUARD_AE_RC_OK, "PING VERSION STATUS HELP") && ok;
    ok = smoke_expect("BOGUS", AMIGUARD_AE_RC_ERROR, "ERROR unknown command") && ok;
    if (!ok) return AMIGUARD_AE_RC_FAIL;
    printf("M1 AROS native dispatcher smoke: PASS\n");
    return AMIGUARD_AE_RC_OK;
#elif defined(__AMIGA__)
    AmiGuardAERexxPort *port;
    unsigned long mask;
    int running = 1;

    port = amiguard_ae_arexx_open();
    if (port == NULL) {
        fprintf(stderr, "%s: cannot open ARexx port %s\n",
                AMIGUARD_AE_NAME, AMIGUARD_AE_AREXX_PORT);
        return AMIGUARD_AE_RC_FAIL;
    }

    printf("%s %s - ARexx port %s ready\n",
           AMIGUARD_AE_NAME, amiguard_ae_version_string(), AMIGUARD_AE_AREXX_PORT);
    mask = amiguard_ae_arexx_signal_mask(port);
    while (running) {
        unsigned long signals = Wait(mask | SIGBREAKF_CTRL_C);
        if ((signals & mask) != 0) amiguard_ae_arexx_process(port);
        if ((signals & SIGBREAKF_CTRL_C) != 0) running = 0;
    }
    amiguard_ae_arexx_close(port);
    return AMIGUARD_AE_RC_OK;
#else
    printf("%s %s\n", AMIGUARD_AE_NAME, amiguard_ae_version_string());
    printf("Host build: native ARexx service is available only on Amiga targets.\n");
    return 0;
#endif
}
