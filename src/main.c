#include <stdio.h>

#include "amiguard_ae.h"

int main(void)
{
    printf("%s %s\n", AMIGUARD_AE_NAME, amiguard_ae_version_string());
    printf("ARexx port: %s (planned for M1)\n", AMIGUARD_AE_AREXX_PORT);
    return 0;
}
