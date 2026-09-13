#include <stdio.h>
#include <string.h>

#include "arexx_dispatch.h"

static int expect(const char *command, long rc, const char *result)
{
    AmiGuardAERexxResult actual;
    amiguard_ae_arexx_dispatch(command, &actual);
    if (actual.rc != rc || strcmp(actual.result, result) != 0) {
        fprintf(stderr, "FAIL command=%s rc=%ld result=%s\n",
                command, actual.rc, actual.result);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failed = 0;
    failed += expect("PING", 0, "PONG");
    failed += expect(" version ", 0, "AmiGuard AE 0.1.0-m1");
    failed += expect("STATUS", 0, "READY M1 scanner=not-connected");
    failed += expect("HELP", 0, "PING VERSION STATUS HELP");
    failed += expect("BOGUS", 10, "ERROR unknown command");
    failed += expect("", 10, "ERROR empty command");
    if (failed != 0) return 1;
    puts("M1 dispatcher qualification: PASS");
    return 0;
}
