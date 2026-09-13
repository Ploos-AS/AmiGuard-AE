#ifndef AMIGUARD_AE_SIGNATURE_BRIDGE_H
#define AMIGUARD_AE_SIGNATURE_BRIDGE_H

typedef unsigned long (*AmiGuardAESignatureCountProvider)(void);

void amiguard_ae_signature_set_count_provider(AmiGuardAESignatureCountProvider provider);
int amiguard_ae_signature_available(void);
unsigned long amiguard_ae_signature_count(void);

#endif
