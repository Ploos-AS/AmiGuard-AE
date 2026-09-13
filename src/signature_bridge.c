#include "signature_bridge.h"

static AmiGuardAESignatureCountProvider count_provider = 0;

#if defined(AMIGUARD_AE_WITH_AMIGUARD)
#include "file_signatures.h"
#include "signatures.h"

static unsigned long builtin_signature_count(void)
{
    unsigned long boot_count = 0UL;
    (void)amiguard_signatures(&boot_count);
    return boot_count + amiguard_file_signature_count();
}
#endif

void amiguard_ae_signature_set_count_provider(AmiGuardAESignatureCountProvider provider)
{
    count_provider = provider;
}

int amiguard_ae_signature_available(void)
{
    if (count_provider != 0) return 1;
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return 1;
#else
    return 0;
#endif
}

unsigned long amiguard_ae_signature_count(void)
{
    if (count_provider != 0) return count_provider();
#if defined(AMIGUARD_AE_WITH_AMIGUARD)
    return builtin_signature_count();
#else
    return 0UL;
#endif
}
