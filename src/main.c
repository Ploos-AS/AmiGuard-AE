#include <stdio.h>

#include "amiguard_ae.h"
#include "arexx_amiga.h"

#if defined(__AMIGA__)
#include <exec/tasks.h>
#include <proto/exec.h>
#endif

int main(void)
{
#if defined(__AMIGA__)
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
