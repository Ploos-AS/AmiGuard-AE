#include "arexx_amiga.h"
#include <stddef.h>

#if defined(__AMIGA__)

#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <stdlib.h>
#include <string.h>

#include "amiguard_ae.h"
#include "arexx_dispatch.h"

struct RxsLib *RexxSysBase;

struct AmiGuardAERexxPort {
    struct MsgPort *port;
};

AmiGuardAERexxPort *amiguard_ae_arexx_open(void)
{
    AmiGuardAERexxPort *wrapper;
    struct MsgPort *port;

    if (FindPort((CONST_STRPTR)AMIGUARD_AE_AREXX_PORT) != NULL) return NULL;

    RexxSysBase = (struct RxsLib *)OpenLibrary((CONST_STRPTR)RXSNAME, 36);
    if (RexxSysBase == NULL) return NULL;

    wrapper = (AmiGuardAERexxPort *)malloc(sizeof(*wrapper));
    if (wrapper == NULL) {
        CloseLibrary((struct Library *)RexxSysBase);
        RexxSysBase = NULL;
        return NULL;
    }

    port = CreateMsgPort();
    if (port == NULL) {
        free(wrapper);
        CloseLibrary((struct Library *)RexxSysBase);
        RexxSysBase = NULL;
        return NULL;
    }

    port->mp_Node.ln_Name = (char *)AMIGUARD_AE_AREXX_PORT;
    port->mp_Node.ln_Pri = 0;
    port->mp_Node.ln_Type = NT_MSGPORT;
    AddPort(port);
    wrapper->port = port;
    return wrapper;
}

void amiguard_ae_arexx_close(AmiGuardAERexxPort *wrapper)
{
    if (wrapper == NULL) return;
    if (wrapper->port != NULL) {
        RemPort(wrapper->port);
        DeleteMsgPort(wrapper->port);
    }
    free(wrapper);
    if (RexxSysBase != NULL) {
        CloseLibrary((struct Library *)RexxSysBase);
        RexxSysBase = NULL;
    }
}

unsigned long amiguard_ae_arexx_signal_mask(const AmiGuardAERexxPort *wrapper)
{
    if (wrapper == NULL || wrapper->port == NULL) return 0UL;
    return 1UL << wrapper->port->mp_SigBit;
}

int amiguard_ae_arexx_process(AmiGuardAERexxPort *wrapper)
{
    struct RexxMsg *msg;
    int count = 0;

    if (wrapper == NULL || wrapper->port == NULL) return -1;

    while ((msg = (struct RexxMsg *)GetMsg(wrapper->port)) != NULL) {
        AmiGuardAERexxResult result;
        const char *command = msg->rm_Args[0] != NULL ? (const char *)msg->rm_Args[0] : "";

        amiguard_ae_arexx_dispatch(command, &result);
        msg->rm_Result1 = result.rc;
        msg->rm_Result2 = 0;
        if ((msg->rm_Action & RXFF_RESULT) != 0 && result.result[0] != '\0') {
            STRPTR arg = CreateArgstring((STRPTR)result.result, (LONG)strlen(result.result));
            if (arg != NULL) msg->rm_Result2 = (LONG)arg;
        }
        ReplyMsg((struct Message *)msg);
        ++count;
    }
    return count;
}

#else

struct AmiGuardAERexxPort { int unused; };
AmiGuardAERexxPort *amiguard_ae_arexx_open(void) { return NULL; }
void amiguard_ae_arexx_close(AmiGuardAERexxPort *port) { (void)port; }
unsigned long amiguard_ae_arexx_signal_mask(const AmiGuardAERexxPort *port)
{ (void)port; return 0UL; }
int amiguard_ae_arexx_process(AmiGuardAERexxPort *port)
{ (void)port; return 0; }

#endif
